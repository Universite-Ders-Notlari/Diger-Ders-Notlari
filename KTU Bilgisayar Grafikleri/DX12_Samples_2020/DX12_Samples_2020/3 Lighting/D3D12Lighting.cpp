//
//	DirectX12 > Lighting : Ambient, Diffuse, Specular Colors	
//

#include <windows.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <string>
#include <wrl.h>
#include "resource.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
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
ComPtr<ID3D12PipelineState>			m_pipelineStatePhong;
ComPtr<ID3D12PipelineState>			m_pipelineStateFlat;
ComPtr<ID3D12GraphicsCommandList>	m_commandList;
ComPtr<ID3D12DescriptorHeap>		m_dsvHeap;
ComPtr<ID3D12Resource>				m_depthStencil;
ComPtr<ID3D12Resource>				m_constantBuffer;
SceneConstantBuffer					m_constantBufferData;
UINT8*								m_pCbvDataBegin = NULL;
//int ConstantBufferPerObjectAlignedSize = (sizeof(SceneConstantBuffer) + 255) & ~255;

// App resources.
ComPtr<ID3D12Resource>				m_vertexBufferCube;
D3D12_VERTEX_BUFFER_VIEW			m_vertexBufferViewCube;
ComPtr<ID3D12Resource>				m_indexBufferCube;
D3D12_INDEX_BUFFER_VIEW				m_indexBufferViewCube;
ComPtr<ID3D12Resource>				m_vertexBufferPlane;
D3D12_VERTEX_BUFFER_VIEW			m_vertexBufferViewPlane;
ComPtr<ID3D12Resource>				m_indexBufferPlane;
D3D12_INDEX_BUFFER_VIEW				m_indexBufferViewPlane;

// Synchronization objects.
UINT								m_frameIndex;
HANDLE								m_fenceEvent;
ComPtr<ID3D12Fence>					m_fence;
UINT64								m_fenceValue;

void OnInit();
void OnUpdate();
void OnRender();
void OnDestroy();
void WaitForPreviousFrame();

void ThrowIfFailed(HRESULT hr);
void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);

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
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount			= FrameCount;
	swapChainDesc.Width					= m_width;
	swapChainDesc.Height				= m_height;
	swapChainDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count		= 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),			// Swap chain needs the queue so that it can force a flush on it.
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
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineStatePhong.Get(), IID_PPV_ARGS(&m_commandList)));

	// Now we execute the command list to upload the initial assets (triangle data)
	m_commandList->Close();

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

		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShaderPhong;
		ComPtr<ID3DBlob> pixelShaderFlat;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PS_Phong", "ps_5_0", compileFlags, 0, &pixelShaderPhong, nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PS_Flat", "ps_5_0", compileFlags, 0, &pixelShaderFlat, nullptr));

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// Describe and create the graphics pipeline state object PSO for pixelShaderPhong.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescPhong = {};
		psoDescPhong.InputLayout				= { inputElementDescs, _countof(inputElementDescs) };
		psoDescPhong.pRootSignature				= m_rootSignature.Get();
		psoDescPhong.VS							= CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDescPhong.PS							= CD3DX12_SHADER_BYTECODE(pixelShaderPhong.Get());
		psoDescPhong.RasterizerState			= CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDescPhong.BlendState					= CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDescPhong.DepthStencilState			= CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDescPhong.SampleMask					= UINT_MAX;
		psoDescPhong.PrimitiveTopologyType		= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDescPhong.NumRenderTargets			= 1;
		psoDescPhong.RTVFormats[0]				= DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDescPhong.DSVFormat					= DXGI_FORMAT_D32_FLOAT;
		psoDescPhong.SampleDesc.Count			= 1;

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDescPhong, IID_PPV_ARGS(&m_pipelineStatePhong)));

		// Describe and create the graphics pipeline state object PSO for pixelShaderFlat.
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescFlat = psoDescPhong;
		psoDescFlat.PS = CD3DX12_SHADER_BYTECODE(pixelShaderFlat.Get());
		//psoDescFlat.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDescFlat, IID_PPV_ARGS(&m_pipelineStateFlat)));
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

	// Create the vertex buffer for Cube (Light Source).
	{
		// Define the geometry for a cube.
		Vertex triangleVertices[] =
		{
			{ XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },

			{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
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
			IID_PPV_ARGS(&m_vertexBufferCube)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBufferCube->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		m_vertexBufferCube->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferViewCube.BufferLocation = m_vertexBufferCube->GetGPUVirtualAddress();
		m_vertexBufferViewCube.StrideInBytes  = sizeof(Vertex);
		m_vertexBufferViewCube.SizeInBytes    = vertexBufferSize;
	}

	// Create the index buffer for Cube (Light Source).
	{
		DWORD indices[] =
		{
			// FRONT
			0, 1, 2,
			0, 2, 3,

			// BACK
			4, 5, 6,
			4, 6, 7,

			// RIGHT
			7, 6, 1,
			7, 1, 0,

			// LEFT
			3, 2, 5,
			3, 5, 4,

			// TOP
			7, 0, 3,
			7, 3, 4,

			// BOTTOM
			5, 2, 1,
			5, 1, 6,
		};

		int IndexBufferSize = sizeof(indices);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_indexBufferCube)));

		// Copy the indices to the index buffer.
		UINT8* pIndexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_indexBufferCube->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
		memcpy(pIndexDataBegin, indices, sizeof(indices));
		m_indexBufferCube->Unmap(0, nullptr);

		// Describe the index buffer view.
		m_indexBufferViewCube.BufferLocation	= m_indexBufferCube->GetGPUVirtualAddress();
		m_indexBufferViewCube.Format			= DXGI_FORMAT_R32_UINT;
		m_indexBufferViewCube.SizeInBytes		= IndexBufferSize;
	}

	// Create the vertex buffer for Plane.
	{
		// Define the geometry for a Plane.
		Vertex triangleVertices[] =
		{
			{ XMFLOAT3( 10.0f, -1.0f,  10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3( 10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-10.0f, -1.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-10.0f, -1.0f,  10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
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
			IID_PPV_ARGS(&m_vertexBufferPlane)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBufferPlane->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		m_vertexBufferPlane->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferViewPlane.BufferLocation = m_vertexBufferPlane->GetGPUVirtualAddress();
		m_vertexBufferViewPlane.StrideInBytes  = sizeof(Vertex);
		m_vertexBufferViewPlane.SizeInBytes    = vertexBufferSize;
	}

	// Create the index buffer for Plane.
	{
		DWORD indices[] =
		{
			0, 1, 2,
			0, 2, 3,
		};

		int IndexBufferSize = sizeof(indices);

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_indexBufferPlane)));

		// Copy the indices to the index buffer.
		UINT8* pIndexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_indexBufferPlane->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
		memcpy(pIndexDataBegin, indices, sizeof(indices));
		m_indexBufferPlane->Unmap(0, nullptr);

		// Describe the index buffer view.
		m_indexBufferViewPlane.BufferLocation	= m_indexBufferPlane->GetGPUVirtualAddress();
		m_indexBufferViewPlane.Format			= DXGI_FORMAT_R32_UINT;
		m_indexBufferViewPlane.SizeInBytes		= IndexBufferSize;
	}

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

		CD3DX12_RANGE readRange(0, 0);				// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
	}

	m_viewport.Width		= static_cast<float>(m_width);
	m_viewport.Height		= static_cast<float>(m_height);
	m_viewport.MaxDepth		= 1.0f;

	m_scissorRect.right		= static_cast<float>(m_width);
	m_scissorRect.bottom	= static_cast<float>(m_height);

	// Initialize the world matrix
	g_World = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( 0.0, 10.0, -20.0, 0.0 );
	XMVECTOR At  = XMVectorSet( 0.0,  0.0,   1.0, 0.0 );
	XMVECTOR Up  = XMVectorSet( 0.0,  1.0,   0.0, 0.0 );
	g_View		 = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1280 / (FLOAT)720, 0.01f, 100.0f);

	m_constantBufferData.mWorld			= XMMatrixTranspose(g_World);
	m_constantBufferData.mView			= XMMatrixTranspose(g_View);
	m_constantBufferData.mProjection	= XMMatrixTranspose(g_Projection);
	m_constantBufferData.mEyePos		= XMFLOAT4(0, 10, -20, 0);
}

// Update frame-based values.
void OnUpdate()
{
	//
	// Constant Buffer Settings for Light Source (Cube)
	//

	rotation += 0.03;

	XMMATRIX mRotate		= XMMatrixRotationY(-rotation);
	XMMATRIX mTranslate		= XMMatrixTranslation(-5.0f, 4.0f, 0.0f);
	XMMATRIX mScale			= XMMatrixScaling(0.3f, 0.3f, 0.3f);
	g_World = mScale * mTranslate * mRotate;

	m_constantBufferData.mWorld			= XMMatrixTranspose(g_World);
	m_constantBufferData.mMeshColor		= XMFLOAT4(1, 1, 1, 1);

	XMVECTOR xmvLightPos	= XMVectorSet(0, 0, 0, 0);
	xmvLightPos				= XMVector3Transform(xmvLightPos, g_World);			// Transform LightPos
	XMStoreFloat4(&m_constantBufferData.mLightPos, xmvLightPos);
		
	memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

	//
	// Constant Buffer Settings for Plane
	//

	m_constantBufferData.mMeshColor		= XMFLOAT4(0, 1, 0, 1);
	m_constantBufferData.mLightColor	= XMFLOAT4(1, 1, 1, 1);

	g_World = XMMatrixIdentity();
	m_constantBufferData.mWorld = XMMatrixTranspose(g_World);

	memcpy(m_pCbvDataBegin + 256, &m_constantBufferData, sizeof(m_constantBufferData));
}


// Render the scene.
void OnRender()
{
	// Command list allocators can only be reset when the associated command lists have finished execution on the GPU; apps should use fences to determine GPU execution progress.
	ThrowIfFailed(m_commandAllocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command list, that command list can then be reset at any time and must be before re-recording.
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineStateFlat.Get()));

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate that the back buffer will be used as a render target.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_commandList->SetPipelineState(m_pipelineStateFlat.Get());

	// Set Cube's Constant Buffer, Vertex, Index Buffers and Render
	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferViewCube);
	m_commandList->IASetIndexBuffer(&m_indexBufferViewCube);
	m_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

	m_commandList->SetPipelineState(m_pipelineStatePhong.Get());

	// Set Plane's Constant Buffer, Vertex, Index Buffers and Render
	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress() + 256);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferViewPlane);
	m_commandList->IASetIndexBuffer(&m_indexBufferViewPlane);
	m_commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

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
	wcex.lpszClassName	= "D3D12Lighting";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);

	if (!RegisterClassEx(&wcex)) return E_FAIL;

	// Create window
	m_hinst = hInstance;
	RECT rc = { 0, 0, 1280, 720 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	m_hwnd = CreateWindow(
				"D3D12Lighting", "DirectX12 > Lighting : Ambient, Diffuse, Specular Colors",
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