//--------------------------------------------------------------------------------------
// File: Tutorial_6.cpp
//
// This application demonstrates simple lighting in the vertex shader
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "resource.h"


//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex_PN 
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
};

struct SimpleVertex_P
{
	XMFLOAT3 Pos;
};


struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 vLightPos[2];
	XMFLOAT4 vLightColor[2];
	XMFLOAT4 vMeshColor;
	XMFLOAT4 vEyePos;
}cBuffer;


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = NULL;
ID3D11DeviceContext*    g_pImmediateContext = NULL;
IDXGISwapChain*         g_pSwapChain = NULL;
ID3D11RenderTargetView* g_pRenderTargetView = NULL;
ID3D11Texture2D*        g_pDepthStencil = NULL;
ID3D11DepthStencilView* g_pDepthStencilView = NULL;
ID3D11VertexShader*     g_pVertexShader_PN = NULL;
ID3D11VertexShader*     g_pVertexShader_P = NULL;
ID3D11PixelShader*      g_pPixelShaderPhong = NULL;
ID3D11PixelShader*      g_pPixelShaderSolid = NULL;
ID3D11InputLayout*      g_pVertexLayout_PN = NULL;
ID3D11InputLayout*      g_pVertexLayout_P = NULL;
ID3D11Buffer*           g_pVertexBuffer = NULL;
ID3D11Buffer*           g_pIndexBuffer = NULL;
ID3D11Buffer*           g_pConstantBuffer = NULL;
XMMATRIX                g_World;
XMMATRIX                g_View;
XMMATRIX                g_Projection;
XMVECTOR				Eye;

ID3D11Buffer*           g_pVertexBufferGround = NULL;
ID3D11Buffer*           g_pIndexBufferGround = NULL;


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();


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
            Render();
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
    g_hWnd = CreateWindow( L"TutorialWindowClass", L"Direct3D 11 Tutorial 6 : Phong Shading", WS_OVERLAPPEDWINDOW,
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
        if( SUCCEEDED( hr ) )            break;
    }
    if( FAILED( hr ) )        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
    pBackBuffer->Release();
    if( FAILED( hr ) )        return hr;

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
    if( FAILED( hr ) )        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );
    if( FAILED( hr ) )        return hr;

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
    hr = CompileShaderFromFile( L"Tutorial_6.fx", "VS_PositionNormal", "vs_4_0", &pVSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_pVertexShader_PN );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
        return hr;
	}

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE( layout );

    // Create the input layout
	hr = g_pd3dDevice->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
                                          pVSBlob->GetBufferSize(), &g_pVertexLayout_PN );
	pVSBlob->Release();
	if( FAILED( hr ) )        return hr;


	// Compile the vertex shader
	ID3DBlob* pVSBlob2 = NULL;
	hr = CompileShaderFromFile(L"Tutorial_6.fx", "VS_Position", "vs_4_0", &pVSBlob2);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob2->GetBufferPointer(), pVSBlob2->GetBufferSize(), NULL, &g_pVertexShader_P);
	if (FAILED(hr))
	{
		pVSBlob2->Release();
		return hr;
	}


	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout2[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	numElements = ARRAYSIZE(layout2);

	// Create the input layout
	hr = g_pd3dDevice->CreateInputLayout(layout2, numElements, pVSBlob2->GetBufferPointer(),
		pVSBlob2->GetBufferSize(), &g_pVertexLayout_P);
	pVSBlob2->Release();
	if (FAILED(hr))        return hr;

    // Set the input layout
    //g_pImmediateContext->IASetInputLayout( g_pVertexLayout );

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile( L"Tutorial_6.fx", "PS_Phong", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShaderPhong );
	pPSBlob->Release();
    if( FAILED( hr ) )        return hr;

	// Compile the pixel shader
	pPSBlob = NULL;
	hr = CompileShaderFromFile( L"Tutorial_6.fx", "PS_Solid", "ps_4_0", &pPSBlob );
    if( FAILED( hr ) )
    {
        MessageBox( NULL,
                    L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return hr;
    }

	// Create the pixel shader
	hr = g_pd3dDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_pPixelShaderSolid );
	pPSBlob->Release();
    if( FAILED( hr ) )
        return hr;

    // Create vertex buffer
	SimpleVertex_P vertices[] =
	{
		XMFLOAT3(-1.0f,  1.0f, -1.0f),
		XMFLOAT3( 1.0f,  1.0f, -1.0f),
		XMFLOAT3( 1.0f,  1.0f,  1.0f),
		XMFLOAT3(-1.0f,  1.0f,  1.0f),
		XMFLOAT3(-1.0f, -1.0f, -1.0f),
		XMFLOAT3( 1.0f, -1.0f, -1.0f),
		XMFLOAT3( 1.0f, -1.0f,  1.0f),
		XMFLOAT3(-1.0f, -1.0f,  1.0f),
	};

    D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( SimpleVertex_P ) * 8;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
    if( FAILED( hr ) ) return hr;

    // Create index buffer
	WORD indices[] =
	{
		3, 1, 0,
		2, 1, 3,

		0, 5, 4,
		1, 5, 0,

		3, 4, 7,
		0, 4, 3,

		1, 6, 5,
		2, 6, 1,

		2, 7, 6,
		3, 7, 2,

		6, 4, 5,
		7, 4, 6,
	};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( WORD ) * 36;        // 36 vertices needed for 12 triangles in a triangle list
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &g_pIndexBuffer );
	if( FAILED( hr ) ) return hr;

	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, NULL, &g_pConstantBuffer );
    if( FAILED( hr ) ) return hr;

	// Vertex buffer for ground
    SimpleVertex_PN verticesGround[] =
    {
	    { XMFLOAT3(  10.0f,  0.0f,  10.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ) },
        { XMFLOAT3(  10.0f,  0.0f, -10.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ) },
        { XMFLOAT3( -10.0f,  0.0f, -10.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ) },
        { XMFLOAT3( -10.0f,  0.0f,  10.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ) },
	};

    D3D11_BUFFER_DESC bdGround;
    bdGround.Usage = D3D11_USAGE_DEFAULT;
    bdGround.ByteWidth = sizeof( SimpleVertex_PN ) * 4;
    bdGround.BindFlags = D3D10_BIND_VERTEX_BUFFER;
    bdGround.CPUAccessFlags = 0;
    bdGround.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA InitDataGround;
    InitDataGround.pSysMem = verticesGround;
    hr = g_pd3dDevice->CreateBuffer( &bdGround, &InitDataGround, &g_pVertexBufferGround );
    if( FAILED( hr ) ) return hr;

    // Index buffer for ground
    DWORD indicesGround[] =
    {
        0,1,2,
        0,2,3,
    };

    bdGround.Usage = D3D11_USAGE_DEFAULT;
    bdGround.ByteWidth = sizeof( DWORD ) * 6;
    bdGround.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bdGround.CPUAccessFlags = 0;
    bdGround.MiscFlags = 0;
    InitDataGround.pSysMem = indicesGround;
    hr = g_pd3dDevice->CreateBuffer( &bdGround, &InitDataGround, &g_pIndexBufferGround );
    if( FAILED( hr ) )        return hr;

	// Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Initialize the world matrices
	g_World = XMMatrixIdentity();

    // Initialize the view matrix
	Eye = XMVectorSet( 0.0f, 10.0f, -18.0f, 0.0f );
	XMVECTOR At = XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	g_View = XMMatrixLookAtLH( Eye, At, Up );

    // Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();

    if( g_pConstantBuffer )		g_pConstantBuffer->Release();
    if( g_pVertexBuffer )		g_pVertexBuffer->Release();
    if( g_pIndexBuffer )		g_pIndexBuffer->Release();
    if( g_pVertexLayout_PN )	g_pVertexLayout_PN->Release();
    if( g_pVertexShader_PN )	g_pVertexShader_PN->Release();
	if (g_pVertexShader_PN)		g_pVertexShader_PN->Release();
	if( g_pVertexShader_P )		g_pVertexShader_P->Release();
    if( g_pPixelShaderSolid )	g_pPixelShaderSolid->Release();
    if( g_pPixelShaderPhong )	g_pPixelShaderPhong->Release();
    if( g_pDepthStencil )		g_pDepthStencil->Release();
    if( g_pDepthStencilView )	g_pDepthStencilView->Release();
    if( g_pRenderTargetView )	g_pRenderTargetView->Release();
    if( g_pSwapChain )			g_pSwapChain->Release();
    if( g_pImmediateContext )	g_pImmediateContext->Release();
    if( g_pd3dDevice )			g_pd3dDevice->Release();
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
void Render()
{
    // Update our time
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

    // Setup our lighting parameters
    XMFLOAT4 vLightPos[2] =
    {
        XMFLOAT4(  0.0f,  0.0f,   0.0f,  1.0f ),
		XMFLOAT4(  0.0f,  0.0f,   0.0f,  1.0f )
    };

    XMFLOAT4 vLightColors[2] =
    {
        XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ),
		XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f )
    };

	XMMATRIX mScale		= XMMatrixScaling( 0.3f, 0.3f, 0.3f );	
	XMMATRIX mTranslate[2];
	XMMATRIX mRotate[2];

	mTranslate[0] = XMMatrixTranslation(-6.0f, 6.0f, 0.0f);
	mRotate[0] = XMMatrixRotationY(-1.5f * t);

	XMVECTOR vLightPos0 = XMLoadFloat4( &vLightPos[0] );
	vLightPos0 = XMVector3Transform( vLightPos0, mScale );
	vLightPos0 = XMVector3Transform( vLightPos0, mTranslate[0] );
	vLightPos0 = XMVector3Transform( vLightPos0, mRotate[0] );
	XMStoreFloat4( &vLightPos[0], vLightPos0 );

	mRotate[1]		= XMMatrixRotationY( 1.5f * t );
	mTranslate[1] = XMMatrixTranslation(-4.0f, 6.0f, 0.0f);
	
	XMVECTOR vLightPos1 = XMLoadFloat4( &vLightPos[1] );
	vLightPos1 = XMVector3Transform( vLightPos1, mScale );
	vLightPos1 = XMVector3Transform( vLightPos1, mTranslate[1] );
	vLightPos1 = XMVector3Transform( vLightPos1, mRotate[1] );
	XMStoreFloat4( &vLightPos[1], vLightPos1 );

    // Clear the back buffer
    float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // red, green, blue, alpha
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );

    // Clear the depth buffer to 1.0 (max depth)
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

	cBuffer.mWorld = XMMatrixTranspose( g_World );
	cBuffer.mView = XMMatrixTranspose( g_View );
	cBuffer.mProjection = XMMatrixTranspose( g_Projection );
	cBuffer.vLightPos[0] = vLightPos[0];
	cBuffer.vLightPos[1] = vLightPos[1];
	cBuffer.vLightColor[0] = vLightColors[0];
	cBuffer.vLightColor[1] = vLightColors[1];
	cBuffer.vMeshColor = XMFLOAT4(0, 0, 0, 0);
	XMStoreFloat4(&cBuffer.vEyePos, Eye);

	// Set vertex buffer
    UINT stride = sizeof( SimpleVertex_P );
    UINT offset = 0;

    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );

	// Set index buffer
	g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

	// Set input payout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout_P);

    // Render each light
    for( int m = 0; m < 2; m++ )
    {
        // Update the world variable to reflect the current light
		cBuffer.mWorld = XMMatrixTranspose(mScale * mTranslate[m] * mRotate[m]);  
		cBuffer.vMeshColor = vLightColors[m];
		g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );

		g_pImmediateContext->VSSetShader( g_pVertexShader_P, NULL, 0 );
		g_pImmediateContext->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
		g_pImmediateContext->PSSetShader( g_pPixelShaderSolid, NULL, 0 );
		g_pImmediateContext->DrawIndexed( 36, 0, 0 );
    }

	// Set vertex buffer
	stride = sizeof(SimpleVertex_PN);
	offset = 0;

	// Set vertex buffer
    g_pImmediateContext->IASetVertexBuffers( 0, 1, &g_pVertexBufferGround, &stride, &offset );

	// Set index buffer
	g_pImmediateContext->IASetIndexBuffer( g_pIndexBufferGround, DXGI_FORMAT_R32_UINT, 0 );

	// Set input payout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout_PN);


	// Rotate ground around the origin
	//g_World = XMMatrixRotationY(t);
	//g_World = XMMatrixRotationX(-XM_PI / 6);

    // Update matrix variables and lighting variables
	cBuffer.mWorld = XMMatrixTranspose( g_World );
	cBuffer.vMeshColor = XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f );
	g_pImmediateContext->UpdateSubresource( g_pConstantBuffer, 0, NULL, &cBuffer, 0, 0 );

    // Render the cube
	g_pImmediateContext->VSSetShader( g_pVertexShader_PN, NULL, 0 );
	g_pImmediateContext->PSSetShader( g_pPixelShaderPhong, NULL, 0 );
	g_pImmediateContext->PSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
	g_pImmediateContext->DrawIndexed( 6, 0, 0 );

    // Present our back buffer to our front buffer
    g_pSwapChain->Present( 0, 0 );
}

