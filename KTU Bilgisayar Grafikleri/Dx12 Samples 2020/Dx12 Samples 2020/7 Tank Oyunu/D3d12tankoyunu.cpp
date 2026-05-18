//
//	DirectX12 > Tank Oyunu 
//  Student Version
//

#include <windows.h>
#include <WindowsX.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <sstream>
#include <vector>
#include <wrl.h>
#include "resource.h"
#include "DDSTextureLoader.h"
#include "dinput.h"
#include <fstream>
using namespace std;
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

typedef struct
{
	float x, y, z;
} VertexType;

typedef struct
{
	int vIndex1, vIndex2, vIndex3;
	int tIndex1, tIndex2, tIndex3;
	int nIndex1, nIndex2, nIndex3;
} FaceType;

struct intersect
{
	float     t		= 0.0F;
	bool isWall		= false;
	bool isEnemy	= false;
} intersected;

vector<intersect> intersections;

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
float speed					= 0.0;
const UINT FrameCount		= 2;
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
ComPtr<ID3D12PipelineState>			m_pipelineState_Textured;
ComPtr<ID3D12PipelineState>			m_pipelineState_Phong;
ComPtr<ID3D12PipelineState>			m_pipelineState_Solid;
ComPtr<ID3D12GraphicsCommandList>	m_commandList;

// Depth/Stencil
ComPtr<ID3D12DescriptorHeap>		m_dsvHeap;
ComPtr<ID3D12Resource>				m_depthStencil;

ComPtr<ID3D12Resource>				m_vertexBuffer_Tank;
D3D12_VERTEX_BUFFER_VIEW			m_vertexBufferView_Tank;
ComPtr<ID3D12Resource>				textureBuffer_Tank;
ComPtr<ID3D12Resource>				textureBufferUploadHeap_Tank;
int vertexCount_Tank				= 0;

ComPtr<ID3D12Resource>				m_vertexBuffer_Missile;
D3D12_VERTEX_BUFFER_VIEW			m_vertexBufferView_Missile;
int vertexCount_Missile				= 0;

ComPtr<ID3D12Resource>				m_vertexBuffer_Ground;
D3D12_VERTEX_BUFFER_VIEW			m_vertexBufferView_Ground;
ComPtr<ID3D12Resource>				textureBuffer_Ground;
ComPtr<ID3D12Resource>				textureBufferUploadHeap_Ground;
int vertexCount_Ground				= 0;

ComPtr<ID3D12Resource>				m_vertexBuffer_Walls;
D3D12_VERTEX_BUFFER_VIEW			m_vertexBufferView_Walls;
ComPtr<ID3D12Resource>				textureBuffer_Walls;
ComPtr<ID3D12Resource>				textureBufferUploadHeap_Walls;
int vertexCount_Walls				= 0;

ComPtr<ID3D12Resource>				m_vertexBuffer_RedDot;
D3D12_VERTEX_BUFFER_VIEW			m_vertexBufferView_RedDot;
int vertexCount_RedDot				= 0;

Vertex* vertices_Tank;
Vertex* vertices_Missile;
Vertex* vertices_Ground;
Vertex* vertices_Walls;
Vertex* vertices_RedDot;

ComPtr<ID3D12Resource>				m_constantBuffer;
SceneConstantBuffer					m_constantBufferData;
UINT8*								m_pCbvDataBegin = NULL;

// Synchronization objects.
UINT								m_frameIndex;
HANDLE								m_fenceEvent;
ComPtr<ID3D12Fence>					m_fence;
UINT64								m_fenceValue;

float mTheta	= 1.5f*XM_PI;
float mPhi		= XM_PIDIV4;
float mRadius	= 20.0f;
POINT mLastMousePos;

XMVECTOR Eye;
XMVECTOR At;
XMVECTOR Up;

XMVECTOR DefaultForward		= XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR DefaultRight		= XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR camForward			= XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
XMVECTOR camRight			= XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

XMVECTOR RedDotPosition		= XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
XMVECTOR Tank_Position		= XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

XMMATRIX camRotationMatrix;

XMMATRIX g_World_Tank;
XMMATRIX g_World_Enemy;
XMMATRIX g_World_Missile;
XMMATRIX g_World_Enemy_Missile;
XMMATRIX g_World_RedDot;

float moveLeftRight		= 0.0f;
float moveBackForward	= 0.0f;
float camYaw			= 0.0f;
float camPitch			= 0.0f;

XMVECTOR Ro_Tank_Missile;
XMVECTOR Rd_Tank_Missile;
XMVECTOR Ro_Enemy_Missile;
XMVECTOR Rd_Enemy_Missile;

XMVECTOR EnemyBarrelDirection = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

float reddotDistance		= 0.0;
float t_Distance			= FLT_MAX;

bool TraceTankMissile		= false;
bool FireTankMissile		= true;

bool FireEnemyMissile		= true;
bool TraceEnemyMissile		= false;

bool renderTank				= true;
bool renderEnemy			= true;

bool renderTankMissile		= true;
bool renderEnemyMissile		= true;

bool PlayTankFireSoundOnce	= true;
bool PlayTankHitSoundOnce	= true;
bool PlayEnemyHitSoundOnce	= true;
bool PlayEnemyFireSoundOnce = true;

bool Animate_Tank			= false;
bool Animate_Enemy			= false;

IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;

DIMOUSESTATE mouseLastState;
LPDIRECTINPUT8 DirectInput;

void OnInit();
void OnUpdate();
void OnRender();

void OnDestroy();
void WaitForPreviousFrame();
bool InitDirectInput(HINSTANCE hInstance);

void				ThrowIfFailed(HRESULT hr);
void				GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
LRESULT				CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HRESULT				InitWindow(HINSTANCE hInstance, int nCmdShow);

Vertex*				Obj_Loader(char* filename, int* verticesCount);
intersect			nearestObject(vector<intersect> intersections);
vector<intersect>	testIntersections(XMVECTOR Ro, XMVECTOR Rd, XMMATRIX g_World_Enemy);
float				IntersectTriangle(XMVECTOR Ro, XMVECTOR Rd, Vertex* verticesModel, int vertexCount, XMMATRIX& g_World);


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
	D3D12_COMMAND_QUEUE_DESC queueDesc	= {};
	queueDesc.Flags						= D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type						= D3D12_COMMAND_LIST_TYPE_DIRECT;

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
		srvHeapDesc.NumDescriptors				= 3;	// Önemli! : Doku sayısı değişince bunu da güncelle
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

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

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

		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain",      "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PS_Textured", "ps_5_0", compileFlags, 0, &pixelShader_Textured, nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PS_Phong",    "ps_5_0", compileFlags, 0, &pixelShader_Phong, nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PS_Solid",    "ps_5_0", compileFlags, 0, &pixelShader_Solid, nullptr));

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescDefault	= {};
		psoDescDefault.InputLayout							= { inputElementDescs, _countof(inputElementDescs) };
		psoDescDefault.pRootSignature						= m_rootSignature.Get();
		psoDescDefault.VS									= CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDescDefault.RasterizerState						= CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDescDefault.BlendState							= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDescDefault.DepthStencilState					= CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDescDefault.SampleMask							= UINT_MAX;
		psoDescDefault.PrimitiveTopologyType				= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDescDefault.NumRenderTargets						= 1;
		psoDescDefault.RTVFormats[0]						= DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDescDefault.DSVFormat							= DXGI_FORMAT_D32_FLOAT;
		psoDescDefault.SampleDesc.Count						= 1;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescTextured = psoDescDefault;
		psoDescTextured.PS = CD3DX12_SHADER_BYTECODE(pixelShader_Textured.Get());
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDescTextured, IID_PPV_ARGS(&m_pipelineState_Textured)));

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescPhong = psoDescDefault;
		psoDescPhong.PS = CD3DX12_SHADER_BYTECODE(pixelShader_Phong.Get());
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDescPhong, IID_PPV_ARGS(&m_pipelineState_Phong)));

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescSolid = psoDescDefault;
		psoDescSolid.PS = CD3DX12_SHADER_BYTECODE(pixelShader_Solid.Get());
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDescSolid, IID_PPV_ARGS(&m_pipelineState_Solid)));
	}

	// Create the depth stencil view.
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc	= {};
		depthStencilDesc.Format							= DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension					= D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags							= D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue		= {};
		depthOptimizedClearValue.Format					= DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth		= 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil	= 0;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_depthStencil)
		));

		m_device->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// Create the Vertex Buffer for Tank
	{	
		// Read .obj file and fill vertex buffer 
		vertices_Tank				= Obj_Loader("Media/Tank.obj", &vertexCount_Tank);

		const UINT vertexBufferSize = vertexCount_Tank * sizeof(Vertex);
		ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer_Tank)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer_Tank->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, &vertices_Tank[0], vertexBufferSize);
		m_vertexBuffer_Tank->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView_Tank.BufferLocation = m_vertexBuffer_Tank->GetGPUVirtualAddress();
		m_vertexBufferView_Tank.StrideInBytes  = sizeof(Vertex);
		m_vertexBufferView_Tank.SizeInBytes    = vertexBufferSize;
	}

	// Get the CBV SRV descriptor size for the current device.
	const UINT cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// Get a handle to the start of the descriptor heap.
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_descriptorHeapHandle(m_descriptorHeap->GetCPUDescriptorHandleForHeapStart());

	// Create the Texture Buffer for Tank 
	{
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_device.Get(), m_commandList.Get(), L"Media/Tank.dds", textureBuffer_Tank, textureBufferUploadHeap_Tank));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping			= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format							= textureBuffer_Tank->GetDesc().Format;
		srvDesc.ViewDimension					= D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip		= 0;
		srvDesc.Texture2D.MipLevels				= textureBuffer_Tank->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp	= 0.0f;

		m_device->CreateShaderResourceView(textureBuffer_Tank.Get(), &srvDesc, m_descriptorHeapHandle);
	}

	// Create the Vertex Buffer for Missile
	{
		// Read .obj file and fill vertex buffer 
		vertices_Missile			= Obj_Loader("Media/Missile.obj", &vertexCount_Missile);

		const UINT vertexBufferSize = vertexCount_Missile * sizeof(Vertex);
		ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer_Missile)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer_Missile->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, &vertices_Missile[0], vertexBufferSize);
		m_vertexBuffer_Missile->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView_Missile.BufferLocation = m_vertexBuffer_Missile->GetGPUVirtualAddress();
		m_vertexBufferView_Missile.StrideInBytes = sizeof(Vertex);
		m_vertexBufferView_Missile.SizeInBytes = vertexBufferSize;
	}

	// Create the Vertex Buffer for Ground
	{
		// Read .obj file and fill vertex buffer 
		vertices_Ground				= Obj_Loader("Media/Ground.obj", &vertexCount_Ground);

		const UINT vertexBufferSize = vertexCount_Ground * sizeof(Vertex);
		ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer_Ground)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer_Ground->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, &vertices_Ground[0], vertexBufferSize);
		m_vertexBuffer_Ground->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView_Ground.BufferLocation = m_vertexBuffer_Ground->GetGPUVirtualAddress();
		m_vertexBufferView_Ground.StrideInBytes = sizeof(Vertex);
		m_vertexBufferView_Ground.SizeInBytes = vertexBufferSize;
	}

	// next descriptor
	m_descriptorHeapHandle.Offset(1, cbvSrvDescriptorSize);

	// Create the Texture Buffer for Ground 
	{
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_device.Get(), m_commandList.Get(), L"Media/Marble.dds", textureBuffer_Ground, textureBufferUploadHeap_Ground));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping			= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format							= textureBuffer_Ground->GetDesc().Format;
		srvDesc.ViewDimension					= D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip		= 0;
		srvDesc.Texture2D.MipLevels				= textureBuffer_Ground->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp	= 0.0f;

		m_device->CreateShaderResourceView(textureBuffer_Ground.Get(), &srvDesc, m_descriptorHeapHandle);
	}

	// Create the Vertex Buffer for Walls
	{
		// Read .obj file and fill vertex buffer 
		vertices_Walls				= Obj_Loader("Media/City.obj", &vertexCount_Walls);

		const UINT vertexBufferSize = vertexCount_Walls * sizeof(Vertex);
		ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer_Walls)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer_Walls->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, &vertices_Walls[0], vertexBufferSize);
		m_vertexBuffer_Walls->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView_Walls.BufferLocation = m_vertexBuffer_Walls->GetGPUVirtualAddress();
		m_vertexBufferView_Walls.StrideInBytes = sizeof(Vertex);
		m_vertexBufferView_Walls.SizeInBytes = vertexBufferSize;
	}

	// next descriptor
	m_descriptorHeapHandle.Offset(1, cbvSrvDescriptorSize);

	// Create the Texture Buffer for Walls 
	{
		ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_device.Get(), m_commandList.Get(), L"Media/City.dds", textureBuffer_Walls, textureBufferUploadHeap_Walls));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping			= D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format							= textureBuffer_Walls->GetDesc().Format;
		srvDesc.ViewDimension					= D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip		= 0;
		srvDesc.Texture2D.MipLevels				= textureBuffer_Walls->GetDesc().MipLevels;
		srvDesc.Texture2D.ResourceMinLODClamp	= 0.0f;

		m_device->CreateShaderResourceView(textureBuffer_Walls.Get(), &srvDesc, m_descriptorHeapHandle);
	}

	// Create the Vertex Buffer for RedDot
	{
		// Read .obj file and fill vertex buffer 
		vertices_RedDot				= Obj_Loader("Media/RedDot.obj", &vertexCount_RedDot);

		const UINT vertexBufferSize = vertexCount_RedDot * sizeof(Vertex);
		ThrowIfFailed(m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer_RedDot)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer_RedDot->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, &vertices_RedDot[0], vertexBufferSize);
		m_vertexBuffer_RedDot->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView_RedDot.BufferLocation = m_vertexBuffer_RedDot->GetGPUVirtualAddress();
		m_vertexBufferView_RedDot.StrideInBytes = sizeof(Vertex);
		m_vertexBufferView_RedDot.SizeInBytes = vertexBufferSize;
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

		// Initialize and map the constant buffers. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		ZeroMemory(&m_constantBufferData, sizeof(m_constantBufferData));

		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
		memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
	}

	m_viewport.Width		= static_cast<float>(m_width);
	m_viewport.Height		= static_cast<float>(m_height);
	m_viewport.MaxDepth		= 1.0f;

	m_scissorRect.right		= static_cast<float>(m_width);
	m_scissorRect.bottom	= static_cast<float>(m_height);

	// Initialize the world matrixs
	g_World					= XMMatrixIdentity();
	g_World_Tank			= XMMatrixIdentity();
	g_World_Enemy			= XMMatrixIdentity();
	g_World_Missile			= XMMatrixIdentity();
	g_World_Enemy_Missile	= XMMatrixIdentity();
	g_World_RedDot			= XMMatrixIdentity();

	// Initialize the view matrix
	Eye = XMVectorSet(0.0f, 2.0, -30.0, 0.0);
	At  = XMVectorSet(0.0f, 0.0,   1.0, 0.0);
	Up  = XMVectorSet(0.0f, 1.0,   0.0, 0.0);
	g_View = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1280 / (FLOAT)720, 0.01f, 1000.0f);

	m_constantBufferData.mWorld			= XMMatrixTranspose(g_World);
	m_constantBufferData.mView			= XMMatrixTranspose(g_View);
	m_constantBufferData.mProjection	= XMMatrixTranspose(g_Projection);

	m_constantBufferData.mLightPos		= XMFLOAT4( 15, 10,    5, 0 );
	m_constantBufferData.mLightColor	= XMFLOAT4(  1,  1,    1, 1 );
	m_constantBufferData.mEyePos		= XMFLOAT4(  0,  2,  -30, 0 );
}


// Update frame-based values.
void OnUpdate()
{
	//
	// Animate Light
	//

	rotation += 0.01;
	XMMATRIX mRotate		= XMMatrixRotationY(rotation);
	XMMATRIX mTranslate		= XMMatrixTranslation(-50.0f, 100.0f, 0.0f);
	XMVECTOR xmvLightPos	= XMVectorSet(0, 0, 0, 0);
	xmvLightPos				= XMVector3Transform(xmvLightPos, mTranslate);
	xmvLightPos				= XMVector3Transform(xmvLightPos, mRotate);
	XMStoreFloat4(&m_constantBufferData.mLightPos, xmvLightPos);


	//
	// Constant Buffer Settings for Ground
	//

	m_constantBufferData.mMeshColor		= XMFLOAT4(0, 1, 0, 1);
	m_constantBufferData.mWorld			= XMMatrixTranspose(XMMatrixIdentity());
	memcpy(m_pCbvDataBegin + 0 * 256, &m_constantBufferData, sizeof(m_constantBufferData));


	//
	// Constant Buffer Settings for Walls
	//

	m_constantBufferData.mMeshColor		= XMFLOAT4(1, 0, 0, 1);
	m_constantBufferData.mWorld			= XMMatrixTranspose(XMMatrixIdentity());
	memcpy(m_pCbvDataBegin + 1 * 256, &m_constantBufferData, sizeof(m_constantBufferData));
	

	//
	// Keyboard / Mouse Interactions
	//

	DIMOUSESTATE mouseCurrState;
	BYTE keyboardState[256];

	DIKeyboard->Acquire();
	DIMouse->Acquire();
	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
	if (keyboardState[DIK_ESCAPE] & 0x80) PostMessage(m_hwnd, WM_DESTROY, 0, 0);

	speed = 0.5;		// Tankın Hız Kaysayısı. Ekran Kartına göre değişebilir.

	if (keyboardState[DIK_W] & 0x80)
	{
		moveBackForward += speed;
	}

	if (keyboardState[DIK_S] & 0x80)
	{
		moveBackForward -= speed;
	}

	if (keyboardState[DIK_A] & 0x80)
	{
		moveLeftRight -= speed;
	}

	if (keyboardState[DIK_D] & 0x80)
	{
		moveLeftRight += speed;
	}

	if (!(keyboardState[DIK_SPACE] & 0x80))					// SPACE is UP
	{

	}

	if (keyboardState[DIK_SPACE] & 0x80)					// SPACE is PRESSED
	{
		if (PlayTankFireSoundOnce)
			PlaySound(TEXT("fire.wav"), NULL, SND_FILENAME | SND_ASYNC);
		PlayTankFireSoundOnce = false;

		FireTankMissile = false;
		TraceTankMissile = true;
	}

	if (!(mouseCurrState.rgbButtons[0] & 0x80))				// Left Button is UP
	{

	}

	if (mouseCurrState.rgbButtons[0] & 0x80)				// Left Button is PRESSED (for Reloading)
	{		
		FireTankMissile			= true;
		TraceTankMissile		= false;

		renderEnemy				= true;
		renderTank				= true;

		renderTankMissile		= true;
		renderEnemyMissile		= true;

		FireEnemyMissile		= true;
		TraceEnemyMissile		= false;

		PlayTankFireSoundOnce	= true;
		PlayTankHitSoundOnce	= true;
	}

	if (!(mouseCurrState.rgbButtons[1] & 0x80))		// Right Button is UP (for Zoom out)
	{

	}

	if (mouseCurrState.rgbButtons[1] & 0x80)		// Right Button is PRESSED (for Zoom in)
	{

	}

	if ((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
	{
		camYaw			+= mouseLastState.lX * 0.001f;
		camPitch		+= mouseCurrState.lY * 0.001f; 
		if (Animate_Tank)	camPitch = 0.0;				// Tank sadece y-ekseninde dönsün
		mouseLastState	= mouseCurrState;
	}

	if (keyboardState[DIK_F] & 0x80)					// Animate Tank
	{
		FireEnemyMissile = false;
		TraceEnemyMissile = true;
	}


	camRotationMatrix	= XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);
	At					= XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	At					= XMVector3Normalize(At);

	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix	= XMMatrixRotationY(camYaw);

	camRight			= XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
	Up					= XMVector3TransformCoord(Up, RotateYTempMatrix);
	camForward			= XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);

	Eye					+= moveLeftRight * camRight;
	Eye					+= moveBackForward * camForward;

	moveLeftRight		= 0.0f;
	moveBackForward		= 0.0f;

	At = Eye + At;

	g_View						= XMMatrixLookAtLH(Eye, At, Up);
	m_constantBufferData.mView	= XMMatrixTranspose(g_View);

	XMFLOAT4 Eye_Float4; XMStoreFloat4(&Eye_Float4, Eye);
	m_constantBufferData.mEyePos	= Eye_Float4;
	

	//
	// Constant Buffer Settings for Tank Model
	//

	Tank_Position					= Eye + 3 * (At - Eye) + XMVectorSet(0, -2, 0, 0);		// Tank Bizden 3 birim ileride 2 birim aşağıda
	XMFLOAT4 Tank_Position_Float4; XMStoreFloat4(&Tank_Position_Float4, Tank_Position);
	XMMATRIX mTranslate_Tank		= XMMatrixTranslation(Tank_Position_Float4.x, Tank_Position_Float4.y, Tank_Position_Float4.z);
	
	g_World_Tank					= camRotationMatrix * mTranslate_Tank;
	m_constantBufferData.mWorld		= XMMatrixTranspose(g_World_Tank);
	memcpy(m_pCbvDataBegin + 2 * 256, &m_constantBufferData, sizeof(m_constantBufferData));

	//
	// Fire and Trace Tank Missile
	//

	Ro_Tank_Missile = Eye + XMVectorSet(0, -1, 0, 0);				// 2 (Eye) - 1 = 1 (Namlunun yüksekliği)
	Rd_Tank_Missile = XMVector3Normalize(At - Eye);
	XMFLOAT4X4 g_World_Missile_4x4;

	if (FireTankMissile)
	{		
		XMVECTOR initialPosition	= Ro_Tank_Missile + (3 + 2.5) * Rd_Tank_Missile;		// Namlunun ucu Tankın merkezinden 2.5 birim ileride
		XMFLOAT4 initialPosition_F4; XMStoreFloat4(&initialPosition_F4, initialPosition);
		g_World_Missile				= g_World_Tank;
		XMStoreFloat4x4(&g_World_Missile_4x4, g_World_Missile);
		g_World_Missile_4x4._41		= initialPosition_F4.x;
		g_World_Missile_4x4._42		= initialPosition_F4.y;
		g_World_Missile_4x4._43		= initialPosition_F4.z;
		g_World_Missile				= XMLoadFloat4x4(&g_World_Missile_4x4);
	}

	XMFLOAT4   Rd_Tank_Missile_Float4; XMStoreFloat4(&Rd_Tank_Missile_Float4, Rd_Tank_Missile);

	if (TraceTankMissile)
	{
		XMStoreFloat4x4(&g_World_Missile_4x4, g_World_Missile);
		g_World_Missile_4x4._41 += 0.5 * Rd_Tank_Missile_Float4.x;
		g_World_Missile_4x4._42 += 0.5 * Rd_Tank_Missile_Float4.y;
		g_World_Missile_4x4._43 += 0.5 * Rd_Tank_Missile_Float4.z;
		g_World_Missile = XMLoadFloat4x4(&g_World_Missile_4x4);	
	}

	//
	// Constant Buffer Settings for Tank Missile
	//

	m_constantBufferData.mWorld		= XMMatrixTranspose(g_World_Missile);
	m_constantBufferData.mMeshColor = XMFLOAT4(0.647, 0.165, 0.165, 1);			// Brown Color
	memcpy(m_pCbvDataBegin + 3 * 256, &m_constantBufferData, sizeof(m_constantBufferData));


	//
	// Tank Missile <> Enemy Intersection Tests
	//

	intersections = testIntersections(Ro_Tank_Missile, Rd_Tank_Missile, g_World_Enemy);
	
	if (intersections.size() > 0)
	{
		intersect nearest				= nearestObject(intersections);
		float RedDot_Distance			= nearest.t;

		XMVECTOR RedDot_Position		= Ro_Tank_Missile + RedDot_Distance * Rd_Tank_Missile;
		XMFLOAT4 RedDot_Position_Float4; XMStoreFloat4(&RedDot_Position_Float4, RedDot_Position);
		XMVECTOR Missile_Position		= XMVectorSet(g_World_Missile_4x4._41, g_World_Missile_4x4._42, g_World_Missile_4x4._43, 0);
		float Missile_RedDot_Distance	= XMVectorGetX(XMVector3Length(RedDot_Position - Missile_Position));

		if ((nearest.isEnemy) && (Missile_RedDot_Distance < 0.5f))
		{
			PlaySound(TEXT("hit.wav"), NULL, SND_FILENAME | SND_ASYNC);
			renderEnemy			= false;
			renderTankMissile	= false;
			if (FireEnemyMissile) renderEnemyMissile = false;	// Enemy henüz ateş etmemişse
		}

		intersections.clear();

		XMMATRIX RedDot_Translation		= XMMatrixTranslation(RedDot_Position_Float4.x, RedDot_Position_Float4.y, RedDot_Position_Float4.z);
		XMMATRIX mScale					= XMMatrixScaling(1 + RedDot_Distance * 0.03, 1 + RedDot_Distance * 0.03, 1 + RedDot_Distance * 0.03);
		g_World_RedDot					= mScale * RedDot_Translation;
	}


	//
	// Constant Buffer Settings for RedDot
	//

	m_constantBufferData.mMeshColor		= XMFLOAT4(1, 1, 0, 1);
	m_constantBufferData.mWorld			= XMMatrixTranspose(g_World_RedDot);
	memcpy(m_pCbvDataBegin + 4 * 256, &m_constantBufferData, sizeof(m_constantBufferData));


	//
	// Constant Buffer Settings for Enemy Tank
	//

	m_constantBufferData.mWorld = XMMatrixTranspose(g_World_Enemy);
	memcpy(m_pCbvDataBegin + 5 * 256, &m_constantBufferData, sizeof(m_constantBufferData));

	// DENEYİN YAPILIŞI KODUNU BURADAN İTİBAREN YAZABİLİRSİNİZ


	//
	// Constant Buffer Settings for Enemy Missile
	//
	m_constantBufferData.mWorld = XMMatrixTranspose(g_World_Enemy_Missile);
	m_constantBufferData.mMeshColor = XMFLOAT4(0.647, 0.165, 0.165, 1);			// Brown Color
	memcpy(m_pCbvDataBegin + 6 * 256, &m_constantBufferData, sizeof(m_constantBufferData));


	// Print Something
	std::wostringstream outs;
	outs << "Missile Position = ( " << g_World_Missile_4x4._41 << ", " << g_World_Missile_4x4._42 << ", " << g_World_Missile_4x4._43 << " )";
	SetWindowText(m_hwnd, outs.str().c_str());
}


// Render the scene.
void OnRender()
{
	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState_Textured.Get()));
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
	m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetPipelineState(m_pipelineState_Textured.Get());

	CD3DX12_GPU_DESCRIPTOR_HANDLE Texture(m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());
	const UINT cbvSrvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	Texture.Offset(1, cbvSrvDescriptorSize);
	m_commandList->SetGraphicsRootDescriptorTable(1, Texture);

	// Render Ground
	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + 0 * 256);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView_Ground);
	m_commandList->DrawInstanced(vertexCount_Ground, 1, 0, 0);

	Texture.Offset(1, cbvSrvDescriptorSize);
	m_commandList->SetGraphicsRootDescriptorTable(1, Texture);

	// Render Walls
	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + 1 * 256);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView_Walls);
	m_commandList->DrawInstanced(vertexCount_Walls, 1, 0, 0);

	m_commandList->SetGraphicsRootDescriptorTable(1, m_descriptorHeap->GetGPUDescriptorHandleForHeapStart());

	if (renderTank)
	{
		// Render Tank
		m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + 2 * 256);
		m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView_Tank);
		m_commandList->DrawInstanced(vertexCount_Tank, 1, 0, 0);
	}

	m_commandList->SetPipelineState(m_pipelineState_Phong.Get());

	if (renderTankMissile)
	{
		// Render Tank Missile
		m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + 3 * 256);
		m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView_Missile);
		m_commandList->DrawInstanced(vertexCount_Missile, 1, 0, 0);
	}

	m_commandList->SetPipelineState(m_pipelineState_Solid.Get());

	// Render RedDot
	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + 4 * 256);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView_RedDot);
	m_commandList->DrawInstanced(vertexCount_RedDot, 1, 0, 0);

	m_commandList->SetPipelineState(m_pipelineState_Textured.Get());

	if (renderEnemy)
	{
		// Render Enemy
		m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + 5 * 256);
		m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView_Tank);
		m_commandList->DrawInstanced(vertexCount_Tank, 1, 0, 0);
	}

	m_commandList->SetPipelineState(m_pipelineState_Phong.Get());

	if (renderEnemyMissile)
	{
		// Render Enemy Missile
		m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + 6 * 256);
		m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView_Missile);
		m_commandList->DrawInstanced(vertexCount_Missile, 1, 0, 0);
	}

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

	InitDirectInput(hInstance);

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
	wcex.lpszClassName	= L"DirectX12TankOyunu";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);

	if (!RegisterClassEx(&wcex)) return E_FAIL;

	// Create window
	m_hinst = hInstance;
	RECT rc = { 0, 0, 1280, 720 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	m_hwnd = CreateWindow(
				L"DirectX12TankOyunu", L"DirectX12 > Tank Oyunu",
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


bool InitDirectInput(HINSTANCE hInstance)
{
	HRESULT HR = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&DirectInput, NULL);
	HR = DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL);
	HR = DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL);
	HR = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	HR = DIKeyboard->SetCooperativeLevel(m_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	HR = DIMouse->SetDataFormat(&c_dfDIMouse);
	HR = DIMouse->SetCooperativeLevel(m_hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
	return true;
}


//
//   http://rastertek.com/tutdx11.html
//  "Tutorial 8: Loading Maya 2011 Models" 
//
Vertex* Obj_Loader(char* filename, int* verticesCount)
{
	ifstream fin;
	char input;

	// Initialize the counts.
	int vertexCount		= 0;
	int textureCount	= 0;
	int normalCount		= 0;
	int faceCount		= 0;

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
	vertices	= new VertexType[vertexCount];
	texcoords	= new VertexType[textureCount];
	normals		= new VertexType[normalCount];
	faces		= new FaceType[faceCount];

	// Initialize the indexes.
	vertexIndex		= 0;
	texcoordIndex	= 0;
	normalIndex		= 0;
	faceIndex		= 0;

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
	Vertex* verticesModel = new Vertex[*verticesCount];

	// Now loop through all the faces and output the three vertices for each face.
	int k = 0;
	for (int i = 0; i<faceIndex; i++)
	{
		vIndex = faces[i].vIndex1 - 1;
		tIndex = faces[i].tIndex1 - 1;
		nIndex = faces[i].nIndex1 - 1;

		verticesModel[k].position.x = vertices[vIndex].x;
		verticesModel[k].position.y = vertices[vIndex].y;
		verticesModel[k].position.z = vertices[vIndex].z;

		verticesModel[k].texture.x = texcoords[tIndex].x;
		verticesModel[k].texture.y = texcoords[tIndex].y;

		verticesModel[k].normal.x = normals[nIndex].x;
		verticesModel[k].normal.y = normals[nIndex].y;
		verticesModel[k].normal.z = normals[nIndex].z;

		vIndex = faces[i].vIndex2 - 1;
		tIndex = faces[i].tIndex2 - 1;
		nIndex = faces[i].nIndex2 - 1;

		k++;

		verticesModel[k].position.x = vertices[vIndex].x;
		verticesModel[k].position.y = vertices[vIndex].y;
		verticesModel[k].position.z = vertices[vIndex].z;

		verticesModel[k].texture.x = texcoords[tIndex].x;
		verticesModel[k].texture.y = texcoords[tIndex].y;

		verticesModel[k].normal.x = normals[nIndex].x;
		verticesModel[k].normal.y = normals[nIndex].y;
		verticesModel[k].normal.z = normals[nIndex].z;

		vIndex = faces[i].vIndex3 - 1;
		tIndex = faces[i].tIndex3 - 1;
		nIndex = faces[i].nIndex3 - 1;

		k++;
		verticesModel[k].position.x = vertices[vIndex].x;
		verticesModel[k].position.y = vertices[vIndex].y;
		verticesModel[k].position.z = vertices[vIndex].z;

		verticesModel[k].texture.x = texcoords[tIndex].x;
		verticesModel[k].texture.y = texcoords[tIndex].y;

		verticesModel[k].normal.x = normals[nIndex].x;
		verticesModel[k].normal.y = normals[nIndex].y;
		verticesModel[k].normal.z = normals[nIndex].z;

		k++;
	}

	// Release the four data structures.
	delete[] vertices;
	delete[] texcoords;
	delete[] normals;
	delete[] faces;

	return verticesModel;
}


intersect nearestObject(vector<intersect> intersections)
{
	float min_distance = FLT_MAX;
	int   min_indis = 0;

	for (int i = 0; i < intersections.size(); i++)
		if (intersections.at(i).t < min_distance)
		{
			min_indis = i;
			min_distance = intersections.at(i).t;
		}

	return intersections.at(min_indis);
}


vector<intersect> testIntersections(XMVECTOR Ro, XMVECTOR Rd, XMMATRIX g_World_Enemy)
{
	float t_Ground = IntersectTriangle(Ro, Rd, vertices_Ground, vertexCount_Ground, XMMatrixIdentity());
	float t_Walls  = IntersectTriangle(Ro, Rd, vertices_Walls, vertexCount_Walls, XMMatrixIdentity());
	float t_Enemy  = IntersectTriangle(Ro, Rd, vertices_Tank, vertexCount_Tank, g_World_Enemy);				// Neden XMMatrixIdentity() değil?

	intersect intersected;

	if (t_Ground > 1.0)
	{
		intersected.t = t_Ground;
		intersections.push_back(intersected);
	}

	if (t_Walls > 1.0)
	{
		intersected.t = t_Walls;
		intersected.isWall = true;
		intersections.push_back(intersected);
	}

	if (t_Enemy > 1.0)
	{
		intersected.t = t_Enemy;
		intersected.isEnemy = true;
		intersections.push_back(intersected);
	}

	return intersections;
}


float IntersectTriangle(XMVECTOR Ro, XMVECTOR Rd, Vertex* verticesModel, int vertexCount, XMMATRIX& g_World)
{
	float min_t = 1000000;
	float t = 0.0;

	XMVECTOR normal, R;
	float s, s1, s2, s3;
	XMVECTOR S, S1, S2, S3;

	for (int i = 0; i < vertexCount; i += 3)
	{

		XMVECTOR v0 = XMLoadFloat3(&verticesModel[i].position);
		XMVECTOR v1 = XMLoadFloat3(&verticesModel[i + 1].position);
		XMVECTOR v2 = XMLoadFloat3(&verticesModel[i + 2].position);

		v0 = XMVector3Transform(v0, g_World);
		v1 = XMVector3Transform(v1, g_World);
		v2 = XMVector3Transform(v2, g_World);

		normal = XMVector3Cross((v1 - v0), (v2 - v0));

		float D = -XMVectorGetX(XMVector3Dot(v0, normal));
		t = -(XMVectorGetX(XMVector3Dot(Ro, normal)) + D) / XMVectorGetX(XMVector3Dot(normal, Rd));

		if (t > 0)
		{
			R = Ro + t * Rd;

			S = XMVector3Cross((v1 - v0), (v2 - v0));
			S1 = XMVector3Cross((R - v0), (v2 - v0));
			S2 = XMVector3Cross((v1 - v0), (R - v0));
			S3 = XMVector3Cross((v1 - R), (v2 - R));

			s = XMVectorGetX(XMVector3Length(S));
			s1 = XMVectorGetX(XMVector3Length(S1));
			s2 = XMVectorGetX(XMVector3Length(S2));
			s3 = XMVectorGetX(XMVector3Length(S3));

			float fark = (float)::abs(s - (s1 + s2 + s3));
			float epsilon = 0.0001F;

			if ((fark <= epsilon) && (t < min_t)) min_t = t;
		}
	}

	if (min_t < 1000000)	return min_t;
	else					return 0.0F;
}