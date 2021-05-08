//--------------------------------------------------------------------------------------
//
// Bu program, MAYA ortamýnda çizilip .obj formatýnda export edilen 
// (doku kaplanmýþ) 3D modeli okuyup ilgili vertex buffera yükler ve çizer.
//
//--------------------------------------------------------------------------------------

#include <windows.h>

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "resource.h"

#include <DXGI.h>
#include <dinput.h>

#include <fstream>
using namespace std;

typedef struct
{
	float x, y, z;
}VertexType;

typedef struct
{
	int vIndex1, vIndex2, vIndex3;
	int tIndex1, tIndex2, tIndex3;
	int nIndex1, nIndex2, nIndex3;
}FaceType;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
	XMFLOAT3 Normal;
};

struct ConstantBuffer
{  
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 vEyePos;
	XMFLOAT4 vLightPos[2];
	XMFLOAT4 vMeshColor;
	XMFLOAT4 vLightColor[2];
}cBuffer;

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE                           g_hInst = NULL;
HWND                                g_hWnd = NULL;
D3D_DRIVER_TYPE                     g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL                   g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*                       g_pd3dDevice = NULL;
ID3D11DeviceContext*                g_pImmediateContext = NULL;
IDXGISwapChain*                     g_pSwapChain = NULL;
ID3D11RenderTargetView*             g_pRenderTargetView = NULL;
ID3D11Texture2D*                    g_pDepthStencil = NULL;
ID3D11DepthStencilView*             g_pDepthStencilView = NULL;
ID3D11VertexShader*                 g_pVertexShader = NULL;
ID3D11PixelShader*                  g_pPixelShader = NULL;
ID3D11PixelShader*                  g_pPixelShader_Textured = NULL;
ID3D11PixelShader*                  g_pPixelShader_Solid = NULL;
ID3D11InputLayout*                  g_pVertexLayout = NULL;
ID3D11Buffer*                       g_pVertexBuffer_Ground = NULL;
ID3D11Buffer*                       g_pVertexBuffer_F_35 = NULL;
ID3D11Buffer*                       g_pVertexBuffer_Ball = NULL;
ID3D11Buffer*                       g_pVertexBuffer_TSilindir = NULL;
ID3D11Buffer*                       g_pVertexBuffer_Light = NULL;
ID3D11Buffer*                       g_pIndexBuffer = NULL;
ID3D11Buffer*                       g_pConstantBuffer = NULL;
ID3D11ShaderResourceView*           g_pTextureTSilindir = NULL;
ID3D11ShaderResourceView*           g_pTextureGround = NULL;
ID3D11ShaderResourceView*           g_pTextureBall = NULL;
ID3D11ShaderResourceView*           g_pTextureF_35 = NULL;
ID3D11SamplerState*                 g_pSamplerLinear = NULL;
XMMATRIX                            g_World;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;

SimpleVertex* verticesGround;
SimpleVertex* verticesF_35;
SimpleVertex* verticesBall;
SimpleVertex* verticesTSilindir;
SimpleVertex* verticesLight;

int vertexCountGround = 0; 
int vertexCountF_35 = 0; 
int vertexCountBall = 0; 
int vertexCountTSilindir = 0; 
int vertexCountLight = 0; 

XMVECTOR Ro ;
XMVECTOR Rd ;


//-------------------------------------------------------------------------------------
// DIRECT INPUT
//-------------------------------------------------------------------------------------
IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;

DIMOUSESTATE mouseLastState;
LPDIRECTINPUT8 DirectInput;

float rot				= 0.01f;
double countsPerSecond	= 0.0;
__int64 CounterStart	= 0;
int frameCount			= 0;
int fps					= 0;
__int64 frameTimeOld	= 0;
double frameTime;

bool InitDirectInput(HINSTANCE hInstance);
void DetectInput(double time);
void UpdateScene(double time);
void StartTimer();
double GetTime();
double GetFrameTime();

//-------------------------------------------------------------------------------------
// FIRST PERSON CAMERA
//-------------------------------------------------------------------------------------

XMVECTOR Eye ;
XMVECTOR At ;
XMVECTOR Up ;

XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR DefaultRight	= XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR camForward		= XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR camRight		= XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

XMMATRIX camRotationMatrix;
XMMATRIX groundWorld;

float moveLeftRight = 0.0f;
float moveBackForward = 0.0f;

float camYaw = 0.0f;
float camPitch = 0.0f;

void UpdateCamera();


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT				InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT				InitDevice();
void				CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void				Render(double time);
SimpleVertex*		Obj_Loader(char* filename, int* verticesCount);


// The code that the function Obj_Loader includes is taken from the tutorial named
// "Tutorial 8: Loading Maya 2011 Models" at http://rastertek.com/tutdx11.html page. 

SimpleVertex* Obj_Loader(char* filename, int* verticesCount)
{
	ifstream fin;
	char input;

	// Initialize the counts.
	int vertexCount = 0;
	int textureCount = 0;
	int normalCount = 0;
	int faceCount = 0;

	// Open the file.
	fin.open(filename);

	// Read from the file and continue to read until the end of the file is reached.
	fin.get(input);
	while (!fin.eof())
	{
		// If the line starts with 'v' then count either the vertex, the texture coordinates, or the normal vector.
		if (input == 'v')
		{
			fin.get(input);
			if (input == ' ') { vertexCount++; }
			if (input == 't') { textureCount++; }
			if (input == 'n') { normalCount++; }
		}

		// If the line starts with 'f' then increment the face count.
		if (input == 'f')
		{
			fin.get(input);
			if (input == ' ') { faceCount++; }
		}

		// Otherwise read in the remainder of the line.
		while (input != '\n')
		{
			fin.get(input);
		}

		// Start reading the beginning of the next line.
		fin.get(input);
	}

	// Close the file.
	fin.close();

	VertexType *vertices, *texcoords, *normals;
	FaceType *faces;
	int vertexIndex, texcoordIndex, normalIndex, faceIndex, vIndex, tIndex, nIndex;
	char input2;

	// Initialize the four data structures.
	vertices = new VertexType[vertexCount];
	texcoords = new VertexType[textureCount];
	normals = new VertexType[normalCount];
	faces = new FaceType[faceCount];

	// Initialize the indexes.
	vertexIndex = 0;
	texcoordIndex = 0;
	normalIndex = 0;
	faceIndex = 0;

	// Open the file.
	fin.open(filename);

	// Read in the vertices, texture coordinates, and normals into the data structures.
	// Important: Also convert to left hand coordinate system since Maya uses right hand coordinate system.
	fin.get(input);
	while (!fin.eof())
	{
		if (input == 'v')
		{
			fin.get(input);

			// Read in the vertices.
			if (input == ' ')
			{
				fin >> vertices[vertexIndex].x >> vertices[vertexIndex].y >> vertices[vertexIndex].z;

				// Invert the Z vertex to change to left hand system.
				vertices[vertexIndex].z = vertices[vertexIndex].z * -1.0f;
				vertexIndex++;
			}

			// Read in the texture uv coordinates.
			if (input == 't')
			{
				fin >> texcoords[texcoordIndex].x >> texcoords[texcoordIndex].y;

				// Invert the V texture coordinates to left hand system.
				texcoords[texcoordIndex].y = 1.0f - texcoords[texcoordIndex].y;
				texcoordIndex++;
			}

			// Read in the normals.
			if (input == 'n')
			{
				fin >> normals[normalIndex].x >> normals[normalIndex].y >> normals[normalIndex].z;

				// Invert the Z normal to change to left hand system.
				normals[normalIndex].z = normals[normalIndex].z * -1.0f;
				normalIndex++;
			}
		}

		// Read in the faces.
		if (input == 'f')
		{
			fin.get(input);
			if (input == ' ')
			{
				// Read the face data in backwards to convert it to a left hand system from right hand system.
				fin >> faces[faceIndex].vIndex3 >> input2 >> faces[faceIndex].tIndex3 >> input2 >> faces[faceIndex].nIndex3
					>> faces[faceIndex].vIndex2 >> input2 >> faces[faceIndex].tIndex2 >> input2 >> faces[faceIndex].nIndex2
					>> faces[faceIndex].vIndex1 >> input2 >> faces[faceIndex].tIndex1 >> input2 >> faces[faceIndex].nIndex1;
				faceIndex++;
			}
		}

		// Read in the remainder of the line.
		while (input != '\n')
		{
			fin.get(input);
		}

		// Start reading the beginning of the next line.
		fin.get(input);
	}

	// Close the file.
	fin.close();

	*verticesCount = faceCount * 3;
	SimpleVertex* verticesModel = new SimpleVertex[*verticesCount];

	// Now loop through all the faces and output the three vertices for each face.
	int k = 0;
	for (int i = 0; i<faceIndex; i++)
	{
		vIndex = faces[i].vIndex1 - 1;
		tIndex = faces[i].tIndex1 - 1;
		nIndex = faces[i].nIndex1 - 1;

		verticesModel[k].Pos.x		= vertices[vIndex].x;
		verticesModel[k].Pos.y		= vertices[vIndex].y;
		verticesModel[k].Pos.z		= vertices[vIndex].z;

		verticesModel[k].Tex.x		= texcoords[tIndex].x;
		verticesModel[k].Tex.y		= texcoords[tIndex].y;

		verticesModel[k].Normal.x	= normals[nIndex].x;
		verticesModel[k].Normal.y	= normals[nIndex].y;
		verticesModel[k].Normal.z	= normals[nIndex].z;

		vIndex = faces[i].vIndex2 - 1;
		tIndex = faces[i].tIndex2 - 1;
		nIndex = faces[i].nIndex2 - 1;
		
		k++;

		verticesModel[k].Pos.x = vertices[vIndex].x;
		verticesModel[k].Pos.y = vertices[vIndex].y;
		verticesModel[k].Pos.z = vertices[vIndex].z;

		verticesModel[k].Tex.x = texcoords[tIndex].x;
		verticesModel[k].Tex.y = texcoords[tIndex].y;

		verticesModel[k].Normal.x = normals[nIndex].x;
		verticesModel[k].Normal.y = normals[nIndex].y;
		verticesModel[k].Normal.z = normals[nIndex].z;

		vIndex = faces[i].vIndex3 - 1;
		tIndex = faces[i].tIndex3 - 1;
		nIndex = faces[i].nIndex3 - 1;

		k++;
		verticesModel[k].Pos.x = vertices[vIndex].x;
		verticesModel[k].Pos.y = vertices[vIndex].y;
		verticesModel[k].Pos.z = vertices[vIndex].z;

		verticesModel[k].Tex.x = texcoords[tIndex].x;
		verticesModel[k].Tex.y = texcoords[tIndex].y;

		verticesModel[k].Normal.x = normals[nIndex].x;
		verticesModel[k].Normal.y = normals[nIndex].y;
		verticesModel[k].Normal.z = normals[nIndex].z;

		k++;
	}

	// Release the four data structures.
	delete[] vertices;
	delete[] texcoords;
	delete[] normals;
	delete[] faces;

	return verticesModel;
}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

	if(!InitDirectInput(hInstance))
	{
		MessageBox(0, L"Direct Input Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
			frameCount++;
			if(GetTime() > 1.0f)
			{
				fps = frameCount;
				frameCount = 0;
				StartTimer();
			}	

			frameTime = GetFrameTime();

			DetectInput(frameTime);

            Render(frameTime);
        }
    }

    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 1280, 720 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"TutorialWindowClass", L"Rendering .obj models exported from MAYA        Press W,A,S,D to move", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
    if( FAILED(hr) )
    {
        if( pErrorBlob != NULL )
            OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        if( pErrorBlob ) pErrorBlob->Release();
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}

bool InitDirectInput(HINSTANCE hInstance)
{
	HRESULT HR = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DirectInput, NULL); 
	HR = DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL);
	HR = DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL);
	HR = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	HR = DIKeyboard->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	HR = DIMouse->SetDataFormat(&c_dfDIMouse);
	HR = DIMouse->SetCooperativeLevel(g_hWnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

	return true;
}

void DetectInput(double time)
{
	DIMOUSESTATE mouseCurrState;
	BYTE keyboardState[256];
	DIKeyboard->Acquire();
	DIMouse->Acquire();
	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	DIKeyboard->GetDeviceState(sizeof(keyboardState),(LPVOID)&keyboardState);
	if(keyboardState[DIK_ESCAPE] & 0x80) PostMessage(g_hWnd, WM_DESTROY, 0, 0);

	float speed = 15.0F * time;

	if(keyboardState[DIK_A] & 0x80)
	{
		moveLeftRight -= speed;
	}

	if(keyboardState[DIK_D] & 0x80)
	{
		moveLeftRight += speed;
	}

	if(keyboardState[DIK_W] & 0x80)
	{
		moveBackForward += speed;
	}

	if(keyboardState[DIK_S] & 0x80)
	{
		moveBackForward -= speed;
	}

	if(mouseCurrState.rgbButtons[0] & 0x80)
	{

	}

	if(mouseCurrState.rgbButtons[1] & 0x80)
	{

	}

	if((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
	{
		camYaw += mouseLastState.lX * 0.001f;

		camPitch += mouseCurrState.lY * 0.001f;

		mouseLastState = mouseCurrState;
	}

	UpdateCamera();

	return;
}

void StartTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);
	countsPerSecond = double(frequencyCount.QuadPart);
	QueryPerformanceCounter(&frequencyCount);
	CounterStart = frequencyCount.QuadPart;
}

double GetTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);

	return double(currentTime.QuadPart-CounterStart)/countsPerSecond;
}

double GetFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;
	QueryPerformanceCounter(&currentTime);
	tickCount = currentTime.QuadPart-frameTimeOld;
	frameTimeOld = currentTime.QuadPart;
	if(tickCount < 0.0f) tickCount = 0.0f;

	return float(tickCount)/countsPerSecond;
}

void UpdateCamera()
{
	camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);
	At = XMVector3TransformCoord(DefaultForward, camRotationMatrix );
	At = XMVector3Normalize(At);

	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(camYaw);

	camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
	Up = XMVector3TransformCoord(Up, RotateYTempMatrix);
	camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);

	Eye += moveLeftRight * camRight;
	Eye += moveBackForward * camForward;	

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;

	At = Eye + At;	

	g_View = XMMatrixLookAtLH( Eye, At, Up );
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof( sd ) );
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
                                            D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext );
        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )
        return hr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, NULL, &g_pDepthStencil );
    if( FAILED( hr ) )
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) )
        return hr;

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

    // Compile the vertex shader
    ID3DBlob* pVSBlob = NULL;
    hr = CompileShaderFromFile( L"RenderingMAYAModels.fx", "VS", "vs_4_0", &pVSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader );
    if( FAILED( hr ) )
    {    
        pVSBlob->Release();
        return hr;
    }

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },

	};
    UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout );
    pVSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Set the input layout
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

    // Compile the pixel shader
    ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile( L"RenderingMAYAModels.fx", "PS_Phong", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader );
    pPSBlob->Release();
    if( FAILED( hr ) )	return hr;

    // Compile the pixel shader Textured
    ID3DBlob* pPStextureBlob = NULL;
    hr = CompileShaderFromFile( L"RenderingMAYAModels.fx", "PS_Textured_Phong", "ps_4_0", &pPStextureBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader( pPStextureBlob->GetBufferPointer(), pPStextureBlob->GetBufferSize(), NULL, &g_pPixelShader_Textured );
    pPStextureBlob->Release();
    if( FAILED( hr ) )	return hr;


    // Compile the pixel shader Solid
    ID3DBlob* pPSsolidBlob = NULL;
    hr = CompileShaderFromFile( L"RenderingMAYAModels.fx", "PS_Solid", "ps_4_0", &pPSsolidBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader( pPSsolidBlob->GetBufferPointer(), pPSsolidBlob->GetBufferSize(), NULL, &g_pPixelShader_Solid );
    pPSsolidBlob->Release();
    if( FAILED( hr ) )	return hr;


	// Read F_35.obj and fill g_pVertexBuffer_F_35 vertex buffer 
	verticesF_35 = Obj_Loader("Models/F_35.obj", &vertexCountF_35);

	D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * vertexCountF_35;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = verticesF_35;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_F_35 );
    if( FAILED( hr ) )     return hr;
	//

	// Read ground.obj and fill g_pVertexBuffer_Ground vertex buffer
	verticesGround = Obj_Loader("Models/ground.obj", &vertexCountGround);

    bd.ByteWidth = sizeof( SimpleVertex ) * vertexCountGround;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = verticesGround;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_Ground );
    if( FAILED( hr ) )     return hr;
	//

	// Read Light.obj and fill vertexCountLight vertex buffer
	verticesLight = Obj_Loader("Models/Light.obj", &vertexCountLight);

    bd.ByteWidth = sizeof( SimpleVertex ) * vertexCountLight;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = verticesLight;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_Light );
    if( FAILED( hr ) )     return hr;
	//

	// Read TSilindir.obj and fill g_pVertexBuffer_TSilindir vertex buffer
	verticesTSilindir = Obj_Loader("Models/TSilindir.obj", &vertexCountTSilindir);

    bd.ByteWidth = sizeof( SimpleVertex ) * vertexCountTSilindir;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = verticesTSilindir;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_TSilindir );
    if( FAILED( hr ) )     return hr;
	//

	// Read ball.obj and fill g_pVertexBuffer_Ball vertex buffer
	verticesBall = Obj_Loader("Models/ball.obj", &vertexCountBall);

    bd.ByteWidth = sizeof( SimpleVertex ) * vertexCountBall;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = verticesBall;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_Ball );
    if( FAILED( hr ) )     return hr;


    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

 
	// Create the constant buffers
    ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pConstantBuffer );
    if( FAILED( hr ) )	return hr;
    
    bd.ByteWidth = sizeof(ConstantBuffer);
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pConstantBuffer );
    if( FAILED( hr ) )	return hr;
    
    bd.ByteWidth = sizeof(ConstantBuffer);
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pConstantBuffer );
    if( FAILED( hr ) )  return hr;

    // Load the Texture ts.jpg
    hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, L"Models/ts.jpg", NULL, NULL, &g_pTextureTSilindir, NULL );
    if( FAILED( hr ) )	return hr;

	// Load the Texture F_35.png
	hr = D3DX11CreateShaderResourceViewFromFile(g_pd3dDevice, L"Models/F_35.png", NULL, NULL, &g_pTextureF_35, NULL);
	if (FAILED(hr))	return hr;

	// Load the Texture ball.png
    hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, L"Models/ball.png", NULL, NULL, &g_pTextureBall, NULL );
    if( FAILED( hr ) )	return hr;


    // Load the Texture grass.jpg
    hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, L"Models/grass.jpg", NULL, NULL, &g_pTextureGround, NULL );
    if( FAILED( hr ) )	return hr;

    // Create the sample state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamplerLinear );
    if( FAILED( hr ) )	return hr;

    // Initialize the world matrices
    g_World = XMMatrixIdentity();

    // Initialize the view matrix
    Eye = XMVectorSet( 0.0f, 15.0f, -50.0f, 0.0f );
    At = XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f );
    Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    g_View = XMMatrixLookAtLH( Eye, At, Up );

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 1000.0f );
    
    cBuffer.mProjection = XMMatrixTranspose( g_Projection );
    g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

    if( g_pSamplerLinear )			g_pSamplerLinear->Release();
    if( g_pTextureTSilindir )		g_pTextureTSilindir->Release();
    if( g_pTextureGround )			g_pTextureGround->Release();
    if( g_pConstantBuffer )			g_pConstantBuffer->Release();
    if( g_pConstantBuffer )			g_pConstantBuffer->Release();
    if( g_pConstantBuffer )			g_pConstantBuffer->Release();
    if( g_pVertexBuffer_Ground )	g_pVertexBuffer_Ground->Release();
	if( g_pVertexBuffer_F_35 )		g_pVertexBuffer_F_35->Release();
	if( g_pVertexBuffer_Ball )		g_pVertexBuffer_Ball->Release();
	if( g_pVertexBuffer_TSilindir ) g_pVertexBuffer_TSilindir->Release();
	if( g_pVertexBuffer_Light )		g_pVertexBuffer_Light->Release();
    if( g_pIndexBuffer )			g_pIndexBuffer->Release();
    if( g_pVertexLayout )			g_pVertexLayout->Release();
    if( g_pVertexShader )			g_pVertexShader->Release();
    if( g_pPixelShader )			g_pPixelShader->Release();
	if( g_pPixelShader )			g_pPixelShader_Solid->Release();
    if( g_pDepthStencil )			g_pDepthStencil->Release();
    if( g_pDepthStencilView )		g_pDepthStencilView->Release();
    if( g_pRenderTargetView )		g_pRenderTargetView->Release();
    if( g_pSwapChain )				g_pSwapChain->Release();
    if( g_pImmediateContext )		g_pImmediateContext->Release();
    if( g_pd3dDevice )				g_pd3dDevice->Release();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render(double time)
{
    // Update our time
	rot += 1.0f * time;
	if(rot > 6.28f) rot = 0.0f;


    // Clear the back buffer
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

    // Clear the depth buffer to 1.0 (max depth)
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );


    cBuffer.mView = XMMatrixTranspose( g_View );
	XMStoreFloat4( &cBuffer.vEyePos, Eye );
    g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );

    XMFLOAT4 vLightPos[2] =
    {
        XMFLOAT4( 0.0f,  0.0f,   0.0f,  1.0f ),
		XMFLOAT4( 0.0f,  0.0f,   0.0f,  1.0f )
    };

    XMFLOAT4 vLightColors[2] =
    {
        XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ),
		XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f )
    };

	cBuffer.vLightColor[0] = vLightColors[0];
	cBuffer.vLightColor[1] = vLightColors[1];
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );

	UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Light, &stride, &offset );

	XMMATRIX mRotate;
	XMMATRIX mTranslate1, mTranslate2, mTranslate3;

	mTranslate1	= XMMatrixTranslation( -10.0f, 0.0f, 0.0f );
	mTranslate2	= XMMatrixTranslation( -15.0f, 20.0f, 10.0f );
	mRotate 	= XMMatrixRotationY( rot );
	
	XMVECTOR vLightPos0 = XMLoadFloat4( &vLightPos[0] );
	vLightPos0 = XMVector3Transform( vLightPos0, mTranslate1 );
	vLightPos0 = XMVector3Transform( vLightPos0, mRotate );
	vLightPos0 = XMVector3Transform( vLightPos0, mTranslate2 );
	XMStoreFloat4( &vLightPos[0], vLightPos0 );

	cBuffer.mWorld = XMMatrixTranspose(mTranslate1 * mRotate * mTranslate2); 
	cBuffer.vLightPos[0] = vLightPos[0];
	cBuffer.vMeshColor = vLightColors[0];
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );

	// Render White Light
	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
	g_pImmediateContext->PSSetShader( g_pPixelShader_Solid, NULL, 0 );
	g_pImmediateContext->PSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
	g_pImmediateContext->Draw( vertexCountLight, 0 );

	mTranslate1	= XMMatrixTranslation(  10.0f, 0.0f, 0.0f );
	mTranslate3	= XMMatrixTranslation(  15.0f, 20.0f, -10.0f );

	XMVECTOR vLightPos1 = XMLoadFloat4( &vLightPos[1] );
	vLightPos1 = XMVector3Transform( vLightPos1, mTranslate1 );
	vLightPos1 = XMVector3Transform( vLightPos1, mRotate );
	vLightPos1 = XMVector3Transform( vLightPos1, mTranslate3 );
	XMStoreFloat4( &vLightPos[1], vLightPos1 );

	cBuffer.mWorld = XMMatrixTranspose(mTranslate1 * mRotate * mTranslate3); 
	cBuffer.vLightPos[1] = vLightPos[1];
	cBuffer.vMeshColor = vLightColors[1];
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );

	// Render Yellow Light
	g_pImmediateContext->Draw( vertexCountLight, 0 );

    // Render Ground	
	g_World = XMMatrixIdentity();
	cBuffer.mWorld = XMMatrixTranspose(g_World);
	cBuffer.vMeshColor = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ); 
	g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Ground, &stride, &offset );
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
    g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
    g_pImmediateContext->PSSetShader( g_pPixelShader_Textured, NULL, 0 );
	g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pConstantBuffer );
	g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pTextureGround );
	g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerLinear );
    g_pImmediateContext->Draw( vertexCountGround, 0 );

	// Render F35
	g_World = XMMatrixTranslation(20.0f, 10.0f, 0.0f) * XMMatrixRotationY(-rot);
	cBuffer.mWorld = XMMatrixTranspose(g_World);
	g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_F_35, &stride, &offset );
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->PSSetShader( g_pPixelShader_Textured, NULL, 0 );
	g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pTextureF_35 );
	g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerLinear );
	g_pImmediateContext->Draw( vertexCountF_35, 0 );

	// Render Ball
	g_World = XMMatrixIdentity();
    cBuffer.mWorld = XMMatrixTranspose( g_World );
	g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Ball, &stride, &offset );
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->PSSetShader( g_pPixelShader_Textured, NULL, 0 );
	g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pTextureBall );
	g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerLinear );
	g_pImmediateContext->Draw( vertexCountBall, 0 );

	// Render TSilindir
	g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_TSilindir, &stride, &offset );
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );
	g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
	g_pImmediateContext->PSSetShader( g_pPixelShader_Textured, NULL, 0 );
	g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pTextureTSilindir );
	g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerLinear );
	g_pImmediateContext->Draw( vertexCountTSilindir, 0 );


	// Present our back buffer to our front buffer
	g_pSwapChain->Present( 0, 0 );
	
}