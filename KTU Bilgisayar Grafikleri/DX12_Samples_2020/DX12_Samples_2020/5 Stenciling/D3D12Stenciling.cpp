//
//	DirectX12 > Stenciling Demo : http://www.d3dcoder.net/d3d12.htm (Chapter 11)
//

#include <windows.h>
#include <WindowsX.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>
#include "resource.h"
#include "DDSTextureLoader.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT2 texture;
};

struct SceneConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 mLightPos;
	XMFLOAT4 mLightColor;
	XMFLOAT4 mEyePos;
	XMFLOAT4 mMeshColor;
};

XMMATRIX g_World;
XMMATRIX g_View;
XMMATRIX g_Projection;

HINSTANCE m_hinst			= NULL;
HWND m_hwnd					= NULL;
UINT m_width				= 1280;
UINT m_height				= 720;
UINT m_rtvDescriptorSize	= 0;
bool m_useWarpDevice		= false;	// Adapter info.
float rotation				= 0.0;
const UINT FrameCount		= 2;

float translation			= 0.1;
XMFLOAT4 Translation		= { 0.0f, 0.0f, 0.0f, 0.0f };
float Pi					= 3.1415926535f;

// Pipeline objects.
D3D12_VIEWPORT						m_viewport;
D3D12_RECT							m_scissorRect;
ComPtr<IDXGISwapChain3>				m_swapChain;
ComPtr<ID3D12Device>				m_device;
ComPtr<ID3D12Resource>				m_renderTargets[FrameCount];
ComPtr<ID3D12CommandAllocator>		m_commandAllocator;
ComPtr<ID3D12CommandQueue>			m_commandQueue;
ComPtr<ID3D12RootSignature>			m_rootSignature;
ComPtr<ID3D12DescriptorHeap>		m_rtvHeap;
ComPtr<ID3D12DescriptorHeap>		m_descriptorHeap;
ComPtr<ID3D12PipelineState>			m_pipelineState_Default;
ComPtr<ID3D12PipelineState>			m_pipelineState_Textured;
ComPtr<ID3D12PipelineState>			m_pipelineState_Phong;
ComPtr<ID3D12PipelineState>			m_pipelineState_Solid;
ComPtr<ID3D12PipelineState>			m_pipelineState_markMirrors;
ComPtr<ID3D12PipelineState>			m_pipelineState_drawReflections;
ComPtr<ID3D12PipelineState>			m_pipelineState_transparent;
ComPtr<ID3D12GraphicsCommandList>	m_commandList;

// Depth/Stencil
ComPtr<ID3D12DescriptorHeap>		m_dsvHeap;
ComPtr<ID3D12Resource>				m_depthStencil;

// App resources.
ComPtr<ID3D12Resource>				m_vertexBuffer_Cube;
D3D12_VERTEX_BUFFER_VIEW			m_vertexBufferView_Cube;
ComPtr<ID3D12Resource>				m_indexBuffer_Cube;
D3D12_INDEX_BUFFER_VIEW				m_indexBufferView_Cube;

ComPtr<ID3D12Resource>				m_vertexBuffer_Mirror_Ground_Wall;
D3D12_VERTEX_BUFFER_VIEW			m_vertexBufferView_Mirror_Ground_Wall;

ComPtr<ID3D12Resource>				m_constantBuffer;
SceneConstantBuffer					m_constantBufferData;
UINT8*								m_pCbvDataBegin = NULL;

// Synchronization objects.
UINT								m_frameIndex;
HANDLE								m_fenceEvent;
ComPtr<ID3D12Fence>					m_fence;
UINT64								m_fenceValue;

//Texture Resources
ComPtr<ID3D12Resource>				textureBuffer;
ComPtr<ID3D12Resource>				textureBufferUploadHeap;

float mTheta	= 1.5f*XM_PI;
float mPhi		= XM_PIDIV4;
float mRadius	= 20.0f;
POINT mLastMousePos;

void OnInit();
void OnUpdate();
void OnRender();
void OnDestroy();
void WaitForPreviousFrame();

void ThrowIfFailed(HRESULT hr);
void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);

void OnMouseDown(WPARAM btnState, int x, int y);
void OnMouseUp(WPARAM btnState, int x, int y);
void OnMouseMove(WPARAM btnState, int x, int y);
float Clamp(float x, float low, float high);

void OnInit()
{
	#if defined(_DEBUG)
		// Enable the D3D12 debug layer.
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
			}
		}
	#endif

	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc		= {};
	swapChainDesc.BufferCount				= FrameCount;
	swapChainDesc.Width						= m_width;
	swapChainDesc.Height					= m_height;
	swapChainDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage				= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect				= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count			= 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),		// Swap chain needs the queue so that it can force a flush on it.
		m_hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc	= {};
		rtvHeapDesc.NumDescriptors				= FrameCount;
		rtvHeapDesc.Type						= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags						= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// Describe and create a depth stencil view (DSV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc	= {};
		dsvHeapDesc.NumDescriptors				= 1;
		dsvHeapDesc.Type						= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags						= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

		// Create the SRV heap.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc	= {};
		srvHeapDesc.NumDescriptors				= 1;
		srvHeapDesc.Flags						= D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		srvHeapDesc.Type						= D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

		ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_descriptorHeap)));
	}

	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
	}

	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	// Create the command list.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState_Textured.Get(), IID_PPV_ARGS(&m_commandList)));

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForPreviousFrame();
	}

	// Graphics root signature.
	{
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		CD3DX12_DESCRIPTOR_RANGE1 range(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[1].InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_PIXEL);

		// create a static sampler
		D3D12_STATIC_SAMPLER_DESC sampler	= {};
		sampler.Filter						= D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU					= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressV					= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressW					= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.MipLODBias					= 0;
		sampler.MaxAnisotropy				= 0;
		sampler.ComparisonFunc				= D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor					= D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD						= 0.0f;
		sampler.MaxLOD						= D3D12_FLOAT32_MAX;
		sampler.ShaderRegister				= 0;
		sampler.RegisterSpace				= 0;
		sampler.ShaderVisibility			= D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader_Textured;
		ComPtr<ID3DBlob> pixelShader_Phong;
		ComPtr<ID3DBlob> pixelShader_Solid;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain",      "vs_5_0", compileFlags, 0, &vertexShader,         nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PS_Textured", "ps_5_0", compileFlags, 0, &pixelShader_Textured, nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PS_Phong",    "ps_5_0", compileFlags, 0, &pixelShader_Phong,    nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PS_Solid",    "ps_5_0", compileFlags, 0, &pixelShader_Solid,    nullptr));

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc_Default			= {};
		psoDesc_Default.InputLayout									= { inputElementDescs, _countof(inputElementDescs) };
		psoDesc_Default.pRootSignature								= m_rootSignature.Get();
		psoDesc_Default.VS											= CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc_Default.RasterizerState								= CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc_Default.BlendState									= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc_Default.DepthStencilState							= CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc_Default.SampleMask									= UINT_MAX;
		psoDesc_Default.PrimitiveTopologyType						= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc_Default.NumRenderTargets							= 1;
		psoDesc_Default.RTVFormats[0]								= DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc_Default.DSVFormat									= DXGI_FORMAT_D32_FLOAT;
		psoDesc_Default.SampleDesc.Count							= 1;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc_Default, IID_PPV_ARGS(&m_pipelineState_Default)));

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc_Textured			= psoDesc_Default;
		psoDesc_Textured.PS											= CD3DX12_SHADER_BYTECODE(pixelShader_Textured.Get());
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc_Textured, IID_PPV_ARGS(&m_pipelineState_Textured)));

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc_Phong			= psoDesc_Default;
		psoDesc_Phong.PS											= CD3DX12_SHADER_BYTECODE(pixelShader_Phong.Get());
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc_Phong, IID_PPV_ARGS(&m_pipelineState_Phong)));

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc_Solid = psoDesc_Default;
		psoDesc_Solid.PS = CD3DX12_SHADER_BYTECODE(pixelShader_Solid.Get());
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc_Solid, IID_PPV_ARGS(&m_pipelineState_Solid)));

		//
		// PSO for marking stencil mirrors.
		//

		CD3DX12_BLEND_DESC mirrorBlendState(D3D12_DEFAULT);
		mirrorBlendState.RenderTarget[0].RenderTargetWriteMask = 0;						// Aynayı sadece stencil buffera yazmak için

		D3D12_DEPTH_STENCIL_DESC mirrorDSS;
		mirrorDSS.DepthEnable		= true;
		mirrorDSS.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ZERO;						// Depth buffera yazmayı engelle. Çünkü diğer cisimlerin yansımasının derinlik değeri aynadan büyük çıkacak.
		mirrorDSS.DepthFunc			= D3D12_COMPARISON_FUNC_LESS;
		mirrorDSS.StencilEnable		= true;
		mirrorDSS.StencilReadMask	= 0xff;
		mirrorDSS.StencilWriteMask	= 0xff;

		mirrorDSS.FrontFace.StencilFailOp		= D3D12_STENCIL_OP_KEEP;
		mirrorDSS.FrontFace.StencilDepthFailOp	= D3D12_STENCIL_OP_KEEP;
		mirrorDSS.FrontFace.StencilPassOp		= D3D12_STENCIL_OP_REPLACE;
		mirrorDSS.FrontFace.StencilFunc			= D3D12_COMPARISON_FUNC_ALWAYS;

		// We are not rendering backfacing polygons, so these settings do not matter.
		mirrorDSS.BackFace.StencilFailOp		= D3D12_STENCIL_OP_KEEP;
		mirrorDSS.BackFace.StencilDepthFailOp	= D3D12_STENCIL_OP_KEEP;
		mirrorDSS.BackFace.StencilPassOp		= D3D12_STENCIL_OP_REPLACE;
		mirrorDSS.BackFace.StencilFunc			= D3D12_COMPARISON_FUNC_ALWAYS;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC markMirrorsPsoDesc	= psoDesc_Textured;
		markMirrorsPsoDesc.BlendState							= mirrorBlendState;		// Mirror is full transparent
		markMirrorsPsoDesc.DepthStencilState					= mirrorDSS;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&markMirrorsPsoDesc, IID_PPV_ARGS(&m_pipelineState_markMirrors)));

		//
		// PSO for stencil reflections.
		//

		D3D12_DEPTH_STENCIL_DESC reflectionsDSS;
		reflectionsDSS.DepthEnable				= true;
		reflectionsDSS.DepthWriteMask			= D3D12_DEPTH_WRITE_MASK_ALL;
		reflectionsDSS.DepthFunc				= D3D12_COMPARISON_FUNC_LESS;
		reflectionsDSS.StencilEnable			= true;
		reflectionsDSS.StencilReadMask			= 0xff;
		reflectionsDSS.StencilWriteMask			= 0xff;

		reflectionsDSS.FrontFace.StencilFailOp		= D3D12_STENCIL_OP_KEEP;
		reflectionsDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		reflectionsDSS.FrontFace.StencilPassOp		= D3D12_STENCIL_OP_KEEP;
		reflectionsDSS.FrontFace.StencilFunc		= D3D12_COMPARISON_FUNC_EQUAL;

		// We are not rendering backfacing polygons, so these settings do not matter.
		reflectionsDSS.BackFace.StencilFailOp		= D3D12_STENCIL_OP_KEEP;
		reflectionsDSS.BackFace.StencilDepthFailOp	= D3D12_STENCIL_OP_KEEP;
		reflectionsDSS.BackFace.StencilPassOp		= D3D12_STENCIL_OP_KEEP;
		reflectionsDSS.BackFace.StencilFunc			= D3D12_COMPARISON_FUNC_EQUAL;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC drawReflectionsPsoDesc		= psoDesc_Textured;
		drawReflectionsPsoDesc.DepthStencilState						= reflectionsDSS;
		drawReflectionsPsoDesc.RasterizerState.CullMode					= D3D12_CULL_MODE_BACK;
		drawReflectionsPsoDesc.RasterizerState.FrontCounterClockwise	= true;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&drawReflectionsPsoDesc, IID_PPV_ARGS(&m_pipelineState_drawReflections)));

	}

	// Create the depth stencil view.
	{
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension			= D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment			= 0;
		depthStencilDesc.Width				= 1280;
		depthStencilDesc.Height				= 720;
		depthStencilDesc.DepthOrArraySize	= 1;
		depthStencilDesc.MipLevels			= 1;
		depthStencilDesc.Format				= DXGI_FORMAT_R24G8_TYPELESS;
		depthStencilDesc.SampleDesc.Count	= 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Layout				= D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags				= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format					= DXGI_FORMAT_D24_UNORM_S8_UINT;
		optClear.DepthStencil.Depth		= 1.0f;
		optClear.DepthStencil.Stencil	= 0;
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(m_depthStencil.GetAddressOf())));

		// Create descriptor to mip level 0 of entire resource using the format of the resource.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags				= D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension		= D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice	= 0;
		m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// Create the vertex buffer for Cube.
	{
		// Define the geometry for a cube.
		Vertex triangleVertices[] =
		{
			//	FRONT	            (Z=-1)               NORMALS                (U,V) HEP AYNI
			{ XMFLOAT3(  1.0,  1.0, -1.0 ), XMFLOAT3( 0.0,  0.0, -1.0), XMFLOAT2( 1.0f, 0.0f ) },
			{ XMFLOAT3(  1.0, -1.0, -1.0 ), XMFLOAT3( 0.0,  0.0, -1.0), XMFLOAT2( 1.0f, 1.0f ) },
			{ XMFLOAT3( -1.0, -1.0, -1.0 ), XMFLOAT3( 0.0,  0.0, -1.0), XMFLOAT2( 0.0f, 1.0f ) },
			{ XMFLOAT3( -1.0,  1.0, -1.0 ), XMFLOAT3( 0.0,  0.0, -1.0), XMFLOAT2( 0.0f, 0.0f ) },

			//	BACK	            (Z=+1)
			{ XMFLOAT3( -1.0,  1.0,  1.0 ), XMFLOAT3( 0.0,  0.0,  1.0), XMFLOAT2( 1.0f, 0.0f ) },
			{ XMFLOAT3( -1.0, -1.0,  1.0 ), XMFLOAT3( 0.0,  0.0,  1.0), XMFLOAT2( 1.0f, 1.0f ) },
			{ XMFLOAT3(  1.0, -1.0,  1.0 ), XMFLOAT3( 0.0,  0.0,  1.0), XMFLOAT2( 0.0f, 1.0f ) },
			{ XMFLOAT3(  1.0,  1.0,  1.0 ), XMFLOAT3( 0.0,  0.0,  1.0), XMFLOAT2( 0.0f, 0.0f ) },

			//	TOP		      (Y=+1)
			{ XMFLOAT3(  1.0,  1.0,  1.0 ), XMFLOAT3( 0.0,  1.0,  0.0), XMFLOAT2( 1.0f, 0.0f ) },
			{ XMFLOAT3(  1.0,  1.0, -1.0 ), XMFLOAT3( 0.0,  1.0,  0.0), XMFLOAT2( 1.0f, 1.0f ) },
			{ XMFLOAT3( -1.0,  1.0, -1.0 ), XMFLOAT3( 0.0,  1.0,  0.0), XMFLOAT2( 0.0f, 1.0f ) },
			{ XMFLOAT3( -1.0,  1.0,  1.0 ), XMFLOAT3( 0.0,  1.0,  0.0), XMFLOAT2( 0.0f, 0.0f ) },

			//	DOWN	      (Y=-1)
			{ XMFLOAT3( -1.0, -1.0,  1.0 ), XMFLOAT3( 0.0, -1.0,  0.0), XMFLOAT2( 1.0f, 0.0f ) },
			{ XMFLOAT3( -1.0, -1.0, -1.0 ), XMFLOAT3( 0.0, -1.0,  0.0), XMFLOAT2( 1.0f, 1.0f ) },
			{ XMFLOAT3(  1.0, -1.0, -1.0 ), XMFLOAT3( 0.0, -1.0,  0.0), XMFLOAT2( 0.0f, 1.0f ) },
			{ XMFLOAT3(  1.0, -1.0,  1.0 ), XMFLOAT3( 0.0, -1.0,  0.0), XMFLOAT2( 0.0f, 0.0f ) },

			//	RIGHT	(X=+1)
			{ XMFLOAT3(  1.0,  1.0,  1.0 ), XMFLOAT3( 1.0,  0.0,  0.0), XMFLOAT2( 1.0f, 0.0f ) },
			{ XMFLOAT3(  1.0, -1.0,  1.0 ), XMFLOAT3( 1.0,  0.0,  0.0), XMFLOAT2( 1.0f, 1.0f ) },
			{ XMFLOAT3(  1.0, -1.0, -1.0 ), XMFLOAT3( 1.0,  0.0,  0.0), XMFLOAT2( 0.0f, 1.0f ) },
			{ XMFLOAT3(  1.0,  1.0, -1.0 ), XMFLOAT3( 1.0,  0.0,  0.0), XMFLOAT2( 0.0f, 0.0f ) },

			//	LEFT	(X=-1)
			{ XMFLOAT3( -1.0,  1.0, -1.0 ), XMFLOAT3(-1.0,  0.0,  0.0), XMFLOAT2( 1.0f, 0.0f ) },
			{ XMFLOAT3( -1.0, -1.0, -1.0 ), XMFLOAT3(-1.0,  0.0,  0.0), XMFLOAT2( 1.0f, 1.0f ) },
			{ XMFLOAT3( -1.0, -1.0,  1.0 ), XMFLOAT3(-1.0,  0.0,  0.0), XMFLOAT2( 0.0f, 1.0f ) },
			{ XMFLOAT3( -1.0,  1.0,  1.0 ), XMFLOAT3(-1.0,  0.0,  0.0), XMFLOAT2( 0.0f, 0.0f ) },
		};

		const UINT vertexBufferSize = sizeof(triangleVertices);

		// Note: using upload heaps to transfer static data like vert buffers is not recommended. Every time the GPU needs it, the upload heap will be marshalled over. 
		// Please read up on Default Heap usage. An upload heap is used here for code simplicity and because there are very few verts to actually transfer.
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer_Cube)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer_Cube->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		m_vertexBuffer_Cube->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView_Cube.BufferLocation = m_vertexBuffer_Cube->GetGPUVirtualAddress();
		m_vertexBufferView_Cube.StrideInBytes  = sizeof(Vertex);
		m_vertexBufferView_Cube.SizeInBytes    = vertexBufferSize;
	}

	// Create the index buffer for Cube.
	{
		DWORD indices[] =
		{
			0, 1, 2,
			0, 2, 3,

			4, 5, 6,
			4, 6, 7,

			8,  9, 10,
			8, 10, 11,

			12, 13, 14,
			12, 14, 15,

			16, 17, 18,
			16, 18, 19,

			20, 21, 22,
			20, 22, 23
		};

		int IndexBufferSize = sizeof(indices);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_indexBuffer_Cube)));

		// Copy the indices to the index buffer.
		UINT8* pIndexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_indexBuffer_Cube->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
		memcpy(pIndexDataBegin, indices, sizeof(indices));
		m_indexBuffer_Cube->Unmap(0, nullptr);

		// Describe the index buffer view.
		m_indexBufferView_Cube.BufferLocation	= m_indexBuffer_Cube->GetGPUVirtualAddress();
		m_indexBufferView_Cube.Format			= DXGI_FORMAT_R32_UINT;
		m_indexBufferView_Cube.SizeInBytes		= IndexBufferSize;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////

	// Create the vertex buffer for Mirror, Ground and Wall.
	{

		//    --------------------
		//   |      |      |      |
		//   | Wall |Mirror| Wall |
		//   |   1  |   2  |  3   |
		//   /-------------------/
		//  /      0 Ground     /
		// /-------------------/

		Vertex triangleVertices[] =
		{
			// 0 - Ground
			{ XMFLOAT3(  9.0,  -2.0,  6.0 ), XMFLOAT3( 0.0,  1.0f,  0.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3(  9.0,  -2.0, -6.0 ), XMFLOAT3( 0.0,  1.0f,  0.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3( -9.0,  -2.0, -6.0 ), XMFLOAT3( 0.0,  1.0f,  0.0 ), XMFLOAT2( 0.0, 0.0 ) },

			{ XMFLOAT3( -9.0,  -2.0,  6.0 ), XMFLOAT3( 0.0,  1.0f,  0.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3(  9.0,  -2.0,  6.0 ), XMFLOAT3( 0.0,  1.0f,  0.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3( -9.0,  -2.0, -6.0 ), XMFLOAT3( 0.0,  1.0f,  0.0 ), XMFLOAT2( 0.0, 0.0 ) },

			// 1 - Wall on the left side of the Mirror
			{ XMFLOAT3( -3.0,   4.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3( -3.0,  -2.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3( -9.0,  -2.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },

			{ XMFLOAT3( -9.0,   4.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3( -3.0,   4.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3( -9.0,  -2.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },

			// 2 - Mirror
			{ XMFLOAT3(  3.0,   4.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3(  3.0,  -2.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3( -3.0,  -2.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },

			{ XMFLOAT3( -3.0,   4.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3(  3.0,   4.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3( -3.0,  -2.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },

			// 3 - Wall on the right side of the Mirror
			{ XMFLOAT3(  9.0,   4.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3(  9.0,  -2.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3(  3.0,  -2.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },

			{ XMFLOAT3(  3.0,   4.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3(  9.0,   4.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },
			{ XMFLOAT3(  3.0,  -2.0,  6.0 ), XMFLOAT3( 0.0,  0.0, -1.0 ), XMFLOAT2( 0.0, 0.0 ) },		
		};

		const UINT vertexBufferSize = sizeof(triangleVertices);

		// Note: using upload heaps to transfer static data like vert buffers is not recommended. Every time the GPU needs it, the upload heap will be marshalled over. 
		// Please read up on Default Heap usage. An upload heap is used here for code simplicity and because there are very few verts to actually transfer.
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer_Mirror_Ground_Wall)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer_Mirror_Ground_Wall->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		m_vertexBuffer_Mirror_Ground_Wall->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView_Mirror_Ground_Wall.BufferLocation = m_vertexBuffer_Mirror_Ground_Wall->GetGPUVirtualAddress();
		m_vertexBufferView_Mirror_Ground_Wall.StrideInBytes  = sizeof(Vertex);
		m_vertexBufferView_Mirror_Ground_Wall.SizeInBytes    = vertexBufferSize;
	}

	// Create the texture buffer ( Frank LUNA's Style ).
	{
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(
			m_device.Get(),
			m_commandList.Get(),
			L"TS.dds",
			textureBuffer,
			textureBufferUploadHeap));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc		= {};
		srvDesc.Shader4ComponentMapping				= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format								= textureBuffer->GetDesc().Format;
		srvDesc.ViewDimension						= D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip			= 0;
		srvDesc.Texture2D.MipLevels					= textureBuffer->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp		= 0.0f;
		m_device->CreateShaderResourceView(textureBuffer.Get(), &srvDesc, m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// Now we execute the command list to upload the initial assets
	m_commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create the constant buffer.
	{
		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer)));

		ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));
		CD3DX12_RANGE readRange(0, 0);
		ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
	}

	m_viewport.Width		= static_cast<float>(m_width);
	m_viewport.Height		= static_cast<float>(m_height);
	m_viewport.MaxDepth		= 1.0f;

	m_scissorRect.right		= static_cast<float>(m_width);
	m_scissorRect.bottom	= static_cast<float>(m_height);

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0, 5.0, -12.0, 0.0);
	XMVECTOR At  = XMVectorSet(0.0, 0.0,   1.0, 0.0);
	XMVECTOR Up  = XMVectorSet(0.0, 1.0,   0.0, 0.0);
	g_View = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1280 / (FLOAT)720, 0.01f, 100.0f);

	m_constantBufferData.mWorld			= XMMatrixTranspose(XMMatrixIdentity());
	m_constantBufferData.mView			= XMMatrixTranspose(g_View);
	m_constantBufferData.mProjection	= XMMatrixTranspose(g_Projection);
	m_constantBufferData.mEyePos		= XMFLOAT4(0,  5,  -12, 0);
}


// Update frame-based values.
void OnUpdate()
{
	// Convert spherical to cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	// Build the view matrix and update contant buffer.
	XMVECTOR pos	= XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up		= XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX view	= XMMatrixLookAtLH(pos, target, up);
	m_constantBufferData.mView = XMMatrixTranspose(view);

	//
	// Constant Buffer Settings for Light Source
	//

	rotation += 0.01;

	XMMATRIX mRotate		= XMMatrixRotationY(rotation);
	XMMATRIX mTranslate		= XMMatrixTranslation(-5.0f, 6.0f, 0.0f);

	XMVECTOR xmvLightPos	= XMVectorSet(0, 0, 0, 0);
	xmvLightPos				= XMVector3Transform(xmvLightPos, mTranslate);
	xmvLightPos				= XMVector3Transform(xmvLightPos, mRotate);
	XMStoreFloat4(&m_constantBufferData.mLightPos, xmvLightPos);
	
	m_constantBufferData.mLightColor	= XMFLOAT4(1, 1, 1, 1);
	m_constantBufferData.mMeshColor		= XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_constantBufferData.mWorld			= XMMatrixTranspose(XMMatrixIdentity());

	memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
	
	//
	// Constant Buffer Settings for Cube
	//

	if (GetAsyncKeyState('A') & 0x8000)
		Translation.x -= translation;

	if (GetAsyncKeyState('D') & 0x8000)
		Translation.x += translation;

	if ((GetAsyncKeyState('W') & 0x8000) && Translation.z < 4.5 ) // < 4.5 : duvarýn içinden geçmesin diye
		Translation.z += translation;

	if (GetAsyncKeyState('S') & 0x8000)
		Translation.z -= translation;

	if ((GetAsyncKeyState('Z') & 0x8000) && Translation.y > -1 ) // < -1 : zeminin altýna inmesin diye
		Translation.y -= translation;

	if (GetAsyncKeyState('X') & 0x8000)
		Translation.y += translation;

	XMMATRIX mTranslation = XMMatrixTranslation(Translation.x, Translation.y, Translation.z);

	m_constantBufferData.mWorld = XMMatrixTranspose(mTranslation);
	memcpy(m_pCbvDataBegin + 1 * 256, &m_constantBufferData, sizeof(m_constantBufferData));

	//
	// Constant Buffer Settings for Shadow of the Cube
	//

	XMVECTOR shadowPlane			= XMVectorSet(0.0f, 1.0f, 0.0f, 2.0f);  // y = -2 plane
	XMVECTOR toLight				= XMVectorSet(m_constantBufferData.mLightPos.x - Translation.x, m_constantBufferData.mLightPos.y - Translation.y, m_constantBufferData.mLightPos.z - Translation.z, 0);
	XMMATRIX S						= XMMatrixShadow(shadowPlane, toLight);
	XMMATRIX shadowOffsetY			= XMMatrixTranslation(0.0f, 0.01f, 0.0f);
	m_constantBufferData.mWorld		= XMMatrixTranspose(mTranslation * S * shadowOffsetY);
	m_constantBufferData.mMeshColor = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	memcpy(m_pCbvDataBegin + 2 * 256, &m_constantBufferData, sizeof(m_constantBufferData));

	//
	// Constant Buffer Settings for Reflection of the Cube
	//

	XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, -1.0f, 6.0f);  // z = 6 plane
	XMMATRIX R = XMMatrixReflect(mirrorPlane);
	m_constantBufferData.mWorld = XMMatrixTranspose(mTranslation * R);

	xmvLightPos = XMVectorSet(0, 0, 0, 0);
	xmvLightPos = XMVector3Transform(xmvLightPos, mTranslate);
	xmvLightPos = XMVector3Transform(xmvLightPos, mRotate);
	xmvLightPos = XMVector3Transform(xmvLightPos, R);
	XMStoreFloat4(&m_constantBufferData.mLightPos, xmvLightPos);

	memcpy(m_pCbvDataBegin + 3 * 256, &m_constantBufferData, sizeof(m_constantBufferData));

	//
	// Constant Buffer Settings for Reflection of the Ground
	//

	m_constantBufferData.mWorld = XMMatrixTranspose(R);
	m_constantBufferData.mMeshColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	memcpy(m_pCbvDataBegin + 4 * 256, &m_constantBufferData, sizeof(m_constantBufferData));

	//
	// Constant Buffer Settings for Reflection of Shadow of the Cube
	//

	m_constantBufferData.mWorld = XMMatrixTranspose(mTranslation * S * shadowOffsetY * R);
	m_constantBufferData.mMeshColor = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	memcpy(m_pCbvDataBegin + 5 * 256, &m_constantBufferData, sizeof(m_constantBufferData));
}


// Render the scene.
void OnRender()
{
	// Command list allocators can only be reset when the associated command lists have finished execution on the GPU; apps should use fences to determine GPU execution progress.
	ThrowIfFailed(m_commandAllocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command list, that command list can then be reset at any time and must be before re-recording.
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState_Textured.Get()));

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_descriptorHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetGraphicsRootDescriptorTable(1, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

	// Render Cube
	m_commandList->SetPipelineState(m_pipelineState_Textured.Get());
	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + 1 * 256);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView_Cube);
	m_commandList->IASetIndexBuffer(&m_indexBufferView_Cube);
	m_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

	// Render Shadow of the Cube
	m_commandList->SetPipelineState(m_pipelineState_Solid.Get());
	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + 2 * 256);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView_Cube);
	m_commandList->IASetIndexBuffer(&m_indexBufferView_Cube);
	m_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

	// Render Ground, Left Wall and Right Wall 
	m_commandList->SetPipelineState(m_pipelineState_Phong.Get());
	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView_Mirror_Ground_Wall);
	m_commandList->DrawInstanced(6, 1, 0, 0);
	m_commandList->DrawInstanced(6, 1, 6, 0);
	m_commandList->DrawInstanced(6, 1, 18, 0);

	// Render Mirror on the Back Buffer & Set regarding pixels to 61 on the Stencil Buffer
	m_commandList->OMSetStencilRef(61);
	m_commandList->SetPipelineState(m_pipelineState_markMirrors.Get());
	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());
	m_commandList->DrawInstanced(6, 1, 12, 0);

	// Render Reflection of the Cube on the Back Buffer that pixels on the Stencil Buffer set to 61
	m_commandList->OMSetStencilRef(61);
	m_commandList->SetPipelineState(m_pipelineState_drawReflections.Get());
	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + 3 * 256);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView_Cube);
	m_commandList->IASetIndexBuffer(&m_indexBufferView_Cube);
	m_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

	// Render Reflection of the Ground on the Back Buffer that pixels on the Stencil Buffer set to 61
	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + 4 * 256);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView_Mirror_Ground_Wall);
	m_commandList->DrawInstanced(6, 1, 0, 0);

	// Render Reflection of Shadow of the Cube on the Back Buffer that pixels on the Stencil Buffer set to 61
	m_commandList->SetPipelineState(m_pipelineState_Solid.Get());
	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + 5 * 256);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView_Cube);
	m_commandList->IASetIndexBuffer(&m_indexBufferView_Cube);
	m_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

	// Indicate that the back buffer will now be used to present.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_commandList->Close());

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(1, 0));

	WaitForPreviousFrame();
}


void WaitForPreviousFrame()
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}


void OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be cleaned up by the destructor.
	WaitForPreviousFrame();

	CloseHandle(m_fenceEvent);
}


_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	InitWindow(hInstance, nCmdShow);

	OnInit();

	// Main message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			OnUpdate();
			OnRender();
		}
	}

	OnDestroy();

	return (int)msg.wParam;
}


HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= "D3D12Stenciling";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);

	if (!RegisterClassEx(&wcex)) return E_FAIL;

	// Create window
	m_hinst = hInstance;
	RECT rc = { 0, 0, 1280, 720 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	m_hwnd = CreateWindow(
				"D3D12Stenciling", "DirectX12 > Stenciling Demo        Use : Mouse, W, A, S, D for interaction",
				WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
				rc.right - rc.left, rc.bottom - rc.top, 
				NULL, NULL, hInstance, NULL);

	if (!m_hwnd) return E_FAIL;

	ShowWindow(m_hwnd, nCmdShow);

	return S_OK;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_LBUTTONDOWN:

		case WM_MBUTTONDOWN:

		case WM_RBUTTONDOWN:
			OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		case WM_LBUTTONUP:

		case WM_MBUTTONUP:

		case WM_RBUTTONUP:
			OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		case WM_MOUSEMOVE:
			OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


_Use_decl_annotations_
void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter.Detach();
}


void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::exception();
	}
}


void OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(m_hwnd);
}


void OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}


void OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(mLastMousePos.x - x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(mLastMousePos.y - y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = Clamp(mPhi, 0.1f, Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = Clamp(mRadius, 3.0f, 40.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}


float Clamp(float x, float low, float high)
{
	return x < low ? low : (x > high ? high : x);
}