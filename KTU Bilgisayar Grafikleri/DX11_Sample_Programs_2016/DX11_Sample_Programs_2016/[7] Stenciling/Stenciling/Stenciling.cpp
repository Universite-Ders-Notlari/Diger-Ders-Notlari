//-----------------------------------------------------------------------------------------
//
// Stenciling Demo : Frank LUNA'nýn "Introduction to 3D Game Programming with Direct3D 11.0" 
//                   isimli kitabýna ait örnek programlardan yararlanýlarak yazýlmýþtýr.
//					 http://www.d3dcoder.net/d3d11.htm
//
//------------------------------------------------------------------------------------------

#include <windows.h>
#include "GameTimer.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "resource.h"

//-----------------------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
};

struct CBNeverChanges
{
    XMMATRIX mView;
};

struct CBChangeOnResize
{
    XMMATRIX mProjection;
};

struct CBChangesEveryFrame
{
    XMMATRIX mWorld;
    XMFLOAT4 vMeshColor;
};

//-----------------------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------------------
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
ID3D11InputLayout*                  g_pVertexLayout = NULL;
ID3D11Buffer*                       g_pVertexBuffer_Box = NULL;
ID3D11Buffer*                       g_pVertexBuffer_Mirror_Ground_Wall = NULL;
ID3D11Buffer*                       g_pIndexBuffer = NULL;
ID3D11Buffer*                       g_pCBNeverChanges = NULL;
ID3D11Buffer*                       g_pCBChangeOnResize = NULL;
ID3D11Buffer*                       g_pCBChangesEveryFrame = NULL;
ID3D11ShaderResourceView*           g_pTextureRV = NULL;
ID3D11SamplerState*                 g_pSamplerLinear = NULL;
XMMATRIX                            g_World_Box;
XMMATRIX                            g_View;
XMMATRIX                            g_Projection;

XMFLOAT3 Box_Translation;


static ID3D11DepthStencilState* MarkMirrorDSS		= 0;
static ID3D11DepthStencilState* DrawReflectionDSS	= 0;
static ID3D11DepthStencilState* NoDoubleBlendDSS    = 0;
ID3D11RasterizerState* CullClockwiseRS				= 0;

// Game Timer 
GameTimer mTimer;
bool mAppPaused = false;


//-----------------------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render(float dt);

//-----------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------------------
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

	mTimer.Reset();

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
			mTimer.Tick();

			if( !mAppPaused )
			{
				//CalculateFrameStats();
				//UpdateScene(mTimer.DeltaTime());	
				Render(mTimer.DeltaTime());
			}
			else
			{
				Sleep(100);
			}

            
        }
    }

    CleanupDevice();

    return ( int )msg.wParam;
}


//-----------------------------------------------------------------------------------------
// Register class and create window
//-----------------------------------------------------------------------------------------
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
    g_hWnd = CreateWindow( L"TutorialWindowClass", L"Direct3D 11 : Stenciling Demo        Press W,A,S,D to move Cube, press R to rotate", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//-----------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DX11
//-----------------------------------------------------------------------------------------
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


//-----------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//-----------------------------------------------------------------------------------------
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
    hr = CompileShaderFromFile( L"Stenciling.fx", "VS", "vs_4_0", &pVSBlob );
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
    hr = CompileShaderFromFile( L"Stenciling.fx", "PS_Solid", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader );
    pPSBlob->Release();
    if( FAILED( hr ) ) return hr;

    // Compile the pixel shader textured
    pPSBlob = NULL;
    hr = CompileShaderFromFile( L"Stenciling.fx", "PS_Textured", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShader_Textured );
    pPSBlob->Release();
    if( FAILED( hr ) ) return hr;

    SimpleVertex verticesBox[] =
    {
		//	FRONT	               (Z=-1)          (U,V) HEP AYNI
		{ XMFLOAT3(  1.0f,  2.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3(  1.0f,  0.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  0.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  2.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },

		//	BACK	               (Z=1)
        { XMFLOAT3( -1.0f,  2.0f,  1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( -1.0f,  0.0f,  1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  0.0f,  1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  2.0f,  1.0f ), XMFLOAT2( 0.0f, 0.0f ) },

		//	TOP		        (Y=1)
		{ XMFLOAT3(  1.0f,  2.0f,  1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3(  1.0f,  2.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  2.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  2.0f,  1.0f ), XMFLOAT2( 0.0f, 0.0f ) },

		//	DOWN	       (Y=-1)
		{ XMFLOAT3( -1.0f,  0.0f,  1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( -1.0f,  0.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  0.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  0.0f,  1.0f ), XMFLOAT2( 0.0f, 0.0f ) },

		//	RIGHT	 (X=1)
		{ XMFLOAT3(  1.0f,  2.0f,  1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3(  1.0f,  0.0f,  1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  0.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3(  1.0f,  2.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },

		//	LEFT	(X=-1)
		{ XMFLOAT3( -1.0f,  2.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
        { XMFLOAT3( -1.0f,  0.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  0.0f,  1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
        { XMFLOAT3( -1.0f,  2.0f,  1.0f ), XMFLOAT2( 0.0f, 0.0f ) },

    };

    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = verticesBox;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_Box );
    if( FAILED( hr ) ) return hr;

    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Box, &stride, &offset );

    // Create index buffer
    WORD indices[] =
    {
        0,1,2,
        0,2,3,

        4,5,6,
        4,6,7,

        8,9,10,
        8,10,11,

        12,13,14,
        12,14,15,

        16,17,18,
        16,18,19,

        20,21,22,
        20,22,23
    };

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( WORD ) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
    if( FAILED( hr ) )
        return hr;

    // Set index buffer
    g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

	//    --------------------
	//   |      |      |      |
    //   | Wall |Mirror| Wall |
	//   |   1  |   2  |  3   |
    //   /-------------------/
    //  /      0 Ground     /
	// /-------------------/

    SimpleVertex verticesMirror_Ground_Wall[] =
    {
		// 0 - Ground
		{ XMFLOAT3(  9.0f,  -2.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3(  9.0f,  -2.0f, -6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( -9.0f,  -2.0f, -6.0f ), XMFLOAT2( 0.0f, 0.0f ) },

		{ XMFLOAT3( -9.0f,  -2.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3(  9.0f,  -2.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },        
        { XMFLOAT3( -9.0f,  -2.0f, -6.0f ), XMFLOAT2( 0.0f, 0.0f ) },

		// 1- Wall on the left side of the Mirror
        { XMFLOAT3( -3.0f,   4.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( -3.0f,  -2.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( -9.0f,  -2.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },

		{ XMFLOAT3( -9.0f,   4.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( -3.0f,   4.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( -9.0f,  -2.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },

		// 2- Mirror
        { XMFLOAT3(  3.0f,   4.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3(  3.0f,  -2.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( -3.0f,  -2.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },

		{ XMFLOAT3( -3.0f,   4.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3(  3.0f,   4.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3( -3.0f,  -2.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },

		// 3- Wall on the right side of the Mirror
        { XMFLOAT3(  9.0f,   4.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3(  9.0f,  -2.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3(  3.0f,  -2.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },

		{ XMFLOAT3(  3.0f,   4.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3(  9.0f,   4.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },
        { XMFLOAT3(  3.0f,  -2.0f,  6.0f ), XMFLOAT2( 0.0f, 0.0f ) },

    };

    ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex ) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = verticesMirror_Ground_Wall;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer_Mirror_Ground_Wall );
    if( FAILED( hr ) ) return hr;

    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Create the constant buffers
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(CBNeverChanges);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBNeverChanges );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangeOnResize);
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBChangeOnResize );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangesEveryFrame);
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pCBChangesEveryFrame );
    if( FAILED( hr ) )
        return hr;

    // Load the Texture
    hr = D3DX11CreateShaderResourceViewFromFile( g_pd3dDevice, L"ts.jpg", NULL, NULL, &g_pTextureRV, NULL );
    if( FAILED( hr ) )
        return hr;

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
    if( FAILED( hr ) )        return hr;

	//
	// Mark Mirror Depth Stencil State
	//

	D3D11_DEPTH_STENCIL_DESC mirrorDesc;
	mirrorDesc.DepthEnable      = true;
	mirrorDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ZERO;
    mirrorDesc.DepthFunc        = D3D11_COMPARISON_LESS; 
    mirrorDesc.StencilEnable    = true;
    mirrorDesc.StencilReadMask  = 0xff;
    mirrorDesc.StencilWriteMask = 0xff;

    mirrorDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

	// We are not rendering backfacing polygons, so these settings do not matter.
    mirrorDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
	mirrorDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_REPLACE;
	mirrorDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;

	g_pd3dDevice->CreateDepthStencilState(&mirrorDesc, &MarkMirrorDSS);

	//
	// Draw Reflection Depth Stencil State
	//

	D3D11_DEPTH_STENCIL_DESC drawReflectionDesc;
	drawReflectionDesc.DepthEnable      = true;
	drawReflectionDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;
	drawReflectionDesc.DepthFunc        = D3D11_COMPARISON_LESS; 
	drawReflectionDesc.StencilEnable    = true;
    drawReflectionDesc.StencilReadMask  = 0xff;
    drawReflectionDesc.StencilWriteMask = 0xff;

	drawReflectionDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.FrontFace.StencilFunc		= D3D11_COMPARISON_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	drawReflectionDesc.BackFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilDepthFailOp	= D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	drawReflectionDesc.BackFace.StencilFunc			= D3D11_COMPARISON_EQUAL;

	g_pd3dDevice->CreateDepthStencilState(&drawReflectionDesc, &DrawReflectionDSS);


	//
	// NoDoubleBlendDSS
	//

	D3D11_DEPTH_STENCIL_DESC noDoubleBlendDesc;
	noDoubleBlendDesc.DepthEnable = true;
	noDoubleBlendDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	noDoubleBlendDesc.DepthFunc = D3D11_COMPARISON_LESS;
	noDoubleBlendDesc.StencilEnable = true;
	noDoubleBlendDesc.StencilReadMask = 0xff;
	noDoubleBlendDesc.StencilWriteMask = 0xff;

	noDoubleBlendDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBlendDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	noDoubleBlendDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	noDoubleBlendDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	g_pd3dDevice->CreateDepthStencilState(&noDoubleBlendDesc, &NoDoubleBlendDSS);


	// Rasterizer State
	D3D11_RASTERIZER_DESC cullClockwiseDesc;
	ZeroMemory(&cullClockwiseDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullClockwiseDesc.FillMode = D3D11_FILL_SOLID;
	cullClockwiseDesc.CullMode = D3D11_CULL_BACK;
	cullClockwiseDesc.FrontCounterClockwise = true;
	cullClockwiseDesc.DepthClipEnable = true;
	g_pd3dDevice->CreateRasterizerState(&cullClockwiseDesc, &CullClockwiseRS);

    // Initialize the world matrices
    g_World_Box = XMMatrixIdentity();

    // Initialize the view matrix
    XMVECTOR Eye = XMVectorSet( 0.0f, 5.0f, -15.0f, 0.0f );
    XMVECTOR At = XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f );
    XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    g_View = XMMatrixLookAtLH( Eye, At, Up );

    CBNeverChanges cbNeverChanges;
    cbNeverChanges.mView = XMMatrixTranspose( g_View );
    g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges, 0, NULL, &cbNeverChanges, 0, 0 );

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f );
    
    CBChangeOnResize cbChangesOnResize;
    cbChangesOnResize.mProjection = XMMatrixTranspose( g_Projection );
    g_pImmediateContext->UpdateSubresource( g_pCBChangeOnResize, 0, NULL, &cbChangesOnResize, 0, 0 );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

    if( g_pSamplerLinear ) g_pSamplerLinear->Release();
    if( g_pTextureRV ) g_pTextureRV->Release();
    if( g_pCBNeverChanges ) g_pCBNeverChanges->Release();
    if( g_pCBChangeOnResize ) g_pCBChangeOnResize->Release();
    if( g_pCBChangesEveryFrame ) g_pCBChangesEveryFrame->Release();
    if( g_pVertexBuffer_Box ) g_pVertexBuffer_Box->Release();
    if( g_pIndexBuffer ) g_pIndexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pPixelShader_Textured ) g_pPixelShader_Textured->Release();
    if( g_pDepthStencil ) g_pDepthStencil->Release();
    if( g_pDepthStencilView ) g_pDepthStencilView->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();
    if( g_pd3dDevice ) g_pd3dDevice->Release();
}


//-----------------------------------------------------------------------------------------
// Called every time the application receives a message
//-----------------------------------------------------------------------------------------
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

		case WM_ACTIVATE:
			if( LOWORD(wParam) == WA_INACTIVE )
			{
				mAppPaused = true;
				mTimer.Stop();
			}
			else
			{
				mAppPaused = false;
				mTimer.Start();
			}
			return 0;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}


//-----------------------------------------------------------------------------------------
// Render a frame
//-----------------------------------------------------------------------------------------
void Render(float dt)
{
	// Update t
	static float t = 0.0f;
    if( g_driverType == D3D_DRIVER_TYPE_REFERENCE )
    {
        t += ( float )XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();
        if( dwTimeStart == 0 )
            dwTimeStart = dwTimeCur;
        t = ( dwTimeCur - dwTimeStart ) / 1000.0f;
    }

   // Clear the back and the stencil buffer
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );
	g_pImmediateContext->OMSetDepthStencilState(0, 0);
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0 );

	// Allow user to move box.
	if( GetAsyncKeyState('A') & 0x8000 )
		Box_Translation.x -= 2.0f*dt;

	if( GetAsyncKeyState('D') & 0x8000 )
		Box_Translation.x += 2.0f*dt;

	if( GetAsyncKeyState('W') & 0x8000 )
		Box_Translation.z += 2.0f*dt;

	if( GetAsyncKeyState('S') & 0x8000 )
		Box_Translation.z -= 2.0f*dt;

	if( GetAsyncKeyState('X') & 0x8000 )
		Box_Translation.y += 2.0f*dt;

	if( GetAsyncKeyState('Z') & 0x8000 )
		Box_Translation.y -= 2.0f*dt;

	XMMATRIX Box_Trans	= XMMatrixTranslation(Box_Translation.x, Box_Translation.y, Box_Translation.z);
	
	if( GetAsyncKeyState('R') & 0x8000 ) 
		g_World_Box = XMMatrixRotationY(t) * Box_Trans;
	else 
		g_World_Box = Box_Trans;
	
	if (g_World_Box(3,1) <= -1.0f)	g_World_Box(3,1) = -1.0f;
	if (g_World_Box(3,2) >=  5.0f)	g_World_Box(3,2) = 5.0f;

    CBChangesEveryFrame cb;	
    cb.mWorld		= XMMatrixTranspose( g_World_Box );
    cb.vMeshColor	= XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    g_pImmediateContext->UpdateSubresource( g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0 );
	
    // Render Box to Back Buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Box, &stride, &offset );
    g_pImmediateContext->VSSetShader( g_pVertexShader, NULL, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pCBNeverChanges );
    g_pImmediateContext->VSSetConstantBuffers( 1, 1, &g_pCBChangeOnResize );
    g_pImmediateContext->VSSetConstantBuffers( 2, 1, &g_pCBChangesEveryFrame );
    g_pImmediateContext->PSSetShader( g_pPixelShader_Textured, NULL, 0 );
    g_pImmediateContext->PSSetConstantBuffers( 2, 1, &g_pCBChangesEveryFrame );
    g_pImmediateContext->PSSetShaderResources( 0, 1, &g_pTextureRV );
    g_pImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerLinear );
    g_pImmediateContext->DrawIndexed( 36, 0, 0 );

	// Render Ground to Back Buffer
    cb.mWorld = XMMatrixIdentity();
    cb.vMeshColor = XMFLOAT4( 0.6f, 0.6f, 0.6f, 1.0f );
    g_pImmediateContext->UpdateSubresource( g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0 );
	g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Mirror_Ground_Wall, &stride, &offset );	
	g_pImmediateContext->Draw( 6, 0 );

	// Render Wall on the Left Side of the Mirror to Back Buffer
    cb.vMeshColor = XMFLOAT4( 0.7f, 0.7f, 0.7f, 1.0f );
    g_pImmediateContext->UpdateSubresource( g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0 );
	g_pImmediateContext->Draw( 6, 6 );

	// Render Wall on the Right Side of the Mirror to Back Buffer
	g_pImmediateContext->Draw( 6, 18 );

	// Render Mirror to Stencil Buffer (61) and Back Buffer
	g_pImmediateContext->OMSetDepthStencilState(MarkMirrorDSS, 61);
	cb.vMeshColor = XMFLOAT4( 0.0f, 0.125f, 0.3f, 1.0f );
    g_pImmediateContext->UpdateSubresource( g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0 );
	g_pImmediateContext->Draw( 6, 12 );

	// Render Shadow of Box
	XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 2.0f); // y = -2 plane
	XMFLOAT4 lightDir = XMFLOAT4(-0.6f, -0.8f, 0.0f, 0.0f);
	XMVECTOR toMainLight = -XMLoadFloat4(&lightDir);
	XMMATRIX S = XMMatrixShadow(shadowPlane, toMainLight);
	XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.005f, 0.0f);

	//g_pImmediateContext->OMSetDepthStencilState(NoDoubleBlendDSS, 0);
	
	cb.mWorld = XMMatrixTranspose(g_World_Box * S * shadowOffsetY);
	cb.vMeshColor = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_Box, &stride, &offset);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->DrawIndexed(36, 0, 0);

	// *****************************************************
	// Render Reflections of Box and Ground to Back Buffer of which pixels were set to 61 in the Stencil Buffer
	XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, -6.0f); // z = 6 plane
	XMMATRIX R = XMMatrixReflect(mirrorPlane);
	g_World_Box = g_World_Box * R;
	cb.mWorld = XMMatrixTranspose( g_World_Box );
	cb.vMeshColor = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
	g_pImmediateContext->UpdateSubresource( g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0 );

	// Cull clockwise triangles for reflection.
	g_pImmediateContext->RSSetState(CullClockwiseRS);

	// Only draw reflection into visible mirror pixels as marked 61 by the stencil buffer. 
	g_pImmediateContext->OMSetDepthStencilState(DrawReflectionDSS, 61);

	// Render Reflection of Box
	g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Box, &stride, &offset );
	g_pImmediateContext->PSSetShader(g_pPixelShader_Textured, NULL, 0);
	g_pImmediateContext->DrawIndexed( 36, 0, 0 );

	// Render Reflection of Ground
	cb.mWorld = XMMatrixTranspose( XMMatrixIdentity() * R);
    cb.vMeshColor = XMFLOAT4( 0.6f, 0.6f, 0.6f, 1.0f );
    g_pImmediateContext->UpdateSubresource( g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0 );
	g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer_Mirror_Ground_Wall, &stride, &offset );	
	g_pImmediateContext->Draw( 6, 0 );

	// Render Reflection of Shadow of Box
	cb.mWorld = XMMatrixTranspose(g_World_Box * S * shadowOffsetY);
	cb.vMeshColor = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	g_pImmediateContext->UpdateSubresource(g_pCBChangesEveryFrame, 0, NULL, &cb, 0, 0);
	g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer_Box, &stride, &offset);
	g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);
	g_pImmediateContext->DrawIndexed(36, 0, 0);


	g_pImmediateContext->RSSetState(0);
	//******************************************************
	
    // Present our back buffer to our front buffer
    g_pSwapChain->Present( 0, 0 );
}