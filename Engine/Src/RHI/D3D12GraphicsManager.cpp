#include "pch.h"
#include "RHI/D3D12GrahpicsManager.h"
#include "Framework/Interface/IApplication.h"
#include "Framework/Common/Buffer.h"
#include "Framework/Common/AssetLoader.h"
#include "Framework/Common/SceneManager.h"



namespace Engine
{
	static void GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter)
	{
		IDXGIAdapter1* pAdapter = nullptr;
		*ppAdapter = nullptr;
		for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &pAdapter); adapterIndex++)
		{
			DXGI_ADAPTER_DESC1 desc;
			pAdapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				// Don't select the Basic Render Driver adapter.
				continue;
			}
			// Check to see if the adapter supports Direct3D 12, but don't create the
			// actual device yet.
			if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
		}
		*ppAdapter = pAdapter;
	}
}

namespace Engine
{
	struct SimpleVertex
	{
		Vector3f position;
		Vector3f color;
	};
	int Engine::D3d12GraphicsManager::Initialize()
	{
		int result = GraphicsManager::Initialize();
		if (!result)
		{
			const GfxConfiguration& config = g_pApp->GetConfiguration();
			vp_ = { 0.0f, 0.0f, static_cast<float>(config.viewport_width_), static_cast<float>(config.viewport_height_), 0.0F, 1.0F };
			rect_ = { 0, 0, static_cast<LONG>(config.viewport_width_), static_cast<LONG>(config.viewport_height_) };
			result = static_cast<int>(CreateGraphicsResources());
		}
		return result;
	}
	void D3d12GraphicsManager::Finalize()
	{
		WaitForPreviousFrame();
	}
	void D3d12GraphicsManager::Tick()
	{
	}
	void D3d12GraphicsManager::Clear()
	{
		HRESULT hr;
		if (FAILED(hr = p_cmdalloc_->Reset())) return;
		if (FAILED(hr = p_cmdlist_->Reset(p_cmdalloc_.Get(), p_plstate_.Get()))) return;
		// Indicate that the back buffer will be used as a render target.
		auto barrier_back_buffer = CD3DX12_RESOURCE_BARRIER::Transition(render_target_arr_[cur_back_buf_index_].Get(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		p_cmdlist_->ResourceBarrier(1, &barrier_back_buffer);
		//TODO:Using the depth stencil buffer
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(p_rtv_heap_->GetCPUDescriptorHandleForHeapStart(), cur_back_buf_index_, rtv_desc_size_);
		p_cmdlist_->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
		// clear the back buffer to a deep blue
		p_cmdlist_->ClearRenderTargetView(rtvHandle, kBackColor, 0, nullptr);
		//p_cmdlist_->ClearDepthStencilView(p_dsv_heap_->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
	void D3d12GraphicsManager::Draw()
	{
		PopulateCommandList();
		RenderBuffers();
		WaitForPreviousFrame();
	}
	bool D3d12GraphicsManager::SetPerFrameShaderParameters()
	{
		memcpy(p_cbv_data_begin_ + cur_back_buf_index_ * kSizeConstantBufferPerFrame, &draw_frame_context_, sizeof(draw_frame_context_));
		return true;
	}
	bool D3d12GraphicsManager::SetPerBatchShaderParameters(int32_t index)
	{
		memcpy(p_cbv_data_begin_ + cur_back_buf_index_ * kSizeConstantBufferPerFrame + kSizePerFrameConstantBuffer + index * kSizePerFrameConstantBuffer,
			&draw_batch_context_, sizeof(cur_back_buf_index_));
		return true;
	}
	HRESULT D3d12GraphicsManager::InitializeBuffers()
	{
		HRESULT hr = S_OK;
		//TODO using kinds of buffer
		//if (FAILED(hr = CreateDepthStencil())) return hr;
		//if (FAILED(hr = CreateConstantBuffer())) return hr;
		//if (FAILED(hr = CreateTextureBuffer())) return hr;
		//if (FAILED(hr = CreateSamplerBuffer())) return hr;
		CreateVertexBuffer();
		if(SUCCEEDED(hr = p_cmdlist_->Close())) {
			ID3D12CommandList* ppCommandLists[] = { p_cmdlist_.Get() };
			p_cmdqueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
			if (FAILED(hr = p_device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(p_fence_.GetAddressOf())))) return hr;
			fence_value_ = 1;
			fence_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
			if (fence_event_ == NULL) {
				hr = HRESULT_FROM_WIN32(GetLastError());
				if (FAILED(hr)) return hr;
			}
			WaitForPreviousFrame();
		}
		return hr;
	}
	HRESULT D3d12GraphicsManager::InitializeShader(const char* vs_filename, const char* fs_filename)
	{
		HRESULT hr = S_OK;
		// load the shaders
		Buffer vertexShader = g_pAssetLoader->OpenAndReadBinarySync(vs_filename);
		Buffer pixelShader = g_pAssetLoader->OpenAndReadBinarySync(fs_filename);
		D3D12_SHADER_BYTECODE vertexShaderByteCode;
		vertexShaderByteCode.pShaderBytecode = vertexShader.GetData();
		vertexShaderByteCode.BytecodeLength = vertexShader.GetDataSize();
		D3D12_SHADER_BYTECODE pixelShaderByteCode;
		pixelShaderByteCode.pShaderBytecode = pixelShader.GetData();
		pixelShaderByteCode.BytecodeLength = pixelShader.GetDataSize();
		// create the input layout object
		D3D12_INPUT_ELEMENT_DESC ied[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		//TODO: Create D3D12_RASTERIZER_DESC D3D12_RENDER_TARGET_BLEND_DESC D3D12_BLEND_DESC D3D12_DEPTH_STENCILOP_DESC D3D12_DEPTH_STENCIL_DESC
		//Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { ied, _countof(ied) };
		psoDesc.pRootSignature = p_rootsig_.Get();
		psoDesc.VS = vertexShaderByteCode;
		psoDesc.PS = pixelShaderByteCode;
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		if(FAILED(hr = p_device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&p_plstate_)))) return hr;
		hr = p_device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, p_cmdalloc_.Get(), 
			p_plstate_.Get(), IID_PPV_ARGS(&p_cmdlist_));
		return hr;
	}
	HRESULT D3d12GraphicsManager::RenderBuffers()
	{
		HRESULT hr;
		// execute the command list
		ID3D12CommandList* ppCommandLists[] = { p_cmdlist_.Get() };
		p_cmdqueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		// swap the back buffer and the front buffer
		hr = p_swapchain->Present(1, 0);
		return hr;
	}
	HRESULT D3d12GraphicsManager::CreateDescriptorHeaps()
	{
		HRESULT hr;
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = kFrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if(FAILED(hr = p_device_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(p_rtv_heap_.GetAddressOf())))) return hr;
		rtv_desc_size_ = p_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		//Create CommandAllocator
		if(FAILED(hr = p_device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(p_cmdalloc_.GetAddressOf())))) return hr;
		return hr;
	}
	HRESULT D3d12GraphicsManager::CreateRenderTarget()
	{
		HRESULT hr;
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(p_rtv_heap_->GetCPUDescriptorHandleForHeapStart());
		// Create a RTV for each frame.
		for (uint32_t i = 0; i < kFrameCount; i++)
		{
			if (hr = p_swapchain->GetBuffer(i, IID_PPV_ARGS(render_target_arr_[i].GetAddressOf()))) return hr;
			p_device_->CreateRenderTargetView(render_target_arr_[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1u, rtv_desc_size_);
		}
		return hr;
	}
	HRESULT D3d12GraphicsManager::CreateVertexBuffer()
	{
		HRESULT hr = S_OK;
		SimpleVertex triangleVertices[] =
		{
			{ { -1.f, 1.f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ {1.f, 1.f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ { 1.f, -1.f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
		};
		const UINT vertexBufferSize = sizeof(triangleVertices);
		auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto res_desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		if(FAILED(hr = p_device_->CreateCommittedResource(&heap_properties,D3D12_HEAP_FLAG_NONE,
			&res_desc,D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&p_vertex_buf_))))
			return hr;
		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
		ThrowIfFailed(p_vertex_buf_->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
		p_vertex_buf_->Unmap(0, nullptr);
		// Initialize the vertex buffer view.
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		vertexBufferView.BufferLocation = p_vertex_buf_->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(SimpleVertex);
		vertexBufferView.SizeInBytes = vertexBufferSize;
		vertex_buf_view_.emplace_back(std::move(vertexBufferView));
		return hr;
	}
	HRESULT Engine::D3d12GraphicsManager::CreateRootSignature()
	{
		HRESULT hr = S_OK;
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		if (FAILED(hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error))) return hr;
		if (FAILED(hr = p_device_->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&p_rootsig_)))) return hr;
		return hr;
	}
	HRESULT Engine::D3d12GraphicsManager::WaitForPreviousFrame()
	{
		HRESULT hr;
		const uint64_t fence = fence_value_;
		//Submit a command to the queue to set a fence point from the GPU side
		if (FAILED(hr = p_cmdqueue_->Signal(p_fence_.Get(), fence))) return hr;
		++fence_value_;
		// Wait until the previous frame is finished.
		if (p_fence_->GetCompletedValue() < fence)
		{
			if (FAILED(hr = p_fence_->SetEventOnCompletion(fence, fence_event_))) return hr;
			WaitForSingleObject(fence_event_, INFINITE);
		}
		cur_back_buf_index_ = p_swapchain->GetCurrentBackBufferIndex();
		return hr;
	}
	HRESULT Engine::D3d12GraphicsManager::PopulateCommandList()
	{
		HRESULT hr =S_OK;
		p_cmdlist_->SetGraphicsRootSignature(p_rootsig_.Get());
		p_cmdlist_->RSSetViewports(1, &vp_);
		p_cmdlist_->RSSetScissorRects(1, &rect_);
		p_cmdlist_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		p_cmdlist_->IASetVertexBuffers(0,1,&vertex_buf_view_[0]);
		p_cmdlist_->DrawInstanced(3, 1, 0, 0);
		auto res_barrier = CD3DX12_RESOURCE_BARRIER::Transition(render_target_arr_[cur_back_buf_index_].Get(),D3D12_RESOURCE_STATE_RENDER_TARGET, 
			D3D12_RESOURCE_STATE_PRESENT);
		p_cmdlist_->ResourceBarrier(1, &res_barrier);
		return hr = p_cmdlist_->Close();
	}
	HRESULT Engine::D3d12GraphicsManager::CreateGraphicsResources()
	{
		HRESULT hr;
#if defined(_DEBUG)
		// Enable the D3D12 debug layer.
			ComPtr<ID3D12Debug> pDebugController;
			if (SUCCEEDED(hr = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController)))) 
				pDebugController->EnableDebugLayer();
#endif
		ComPtr<IDXGIFactory4> factory;
		ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(factory.GetAddressOf())));
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), hardwareAdapter.GetAddressOf());
		if (FAILED(hr = D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(p_device_.GetAddressOf()))))
		{
			IDXGIAdapter* pWarpAdapter;
			if (FAILED(hr = factory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter))))
				return hr;
		}
		ThrowIfFailed(hr = D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(p_device_.GetAddressOf())));
		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		ThrowIfFailed(hr = p_device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(p_cmdqueue_.GetAddressOf())));
		HWND hwnd = static_cast<HWND>(g_pApp->GetMainWindowHandler());
		//swap chain descriptor
		// Describe and create the swap chain.
		DXGI_SWAP_CHAIN_DESC scd = {};
		scd.BufferDesc.Width = g_pApp->GetConfiguration().viewport_width_;
		scd.BufferDesc.Height = g_pApp->GetConfiguration().viewport_height_;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.BufferCount = kFrameCount;
		scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // DXGI_SWAP_EFFECT_FLIP_DISCARD only supported after Win10
		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		scd.OutputWindow = hwnd;
		scd.Windowed = true;
		ComPtr<IDXGISwapChain> swapChain;
		// Swap chain needs the queue so that it can force a flush on it.
		hr = factory->CreateSwapChain(p_cmdqueue_.Get(), &scd, swapChain.GetAddressOf());
		ThrowIfFailed(hr = swapChain.As(&p_swapchain));
		cur_back_buf_index_ = p_swapchain->GetCurrentBackBufferIndex();
		if (FAILED(hr = CreateDescriptorHeaps())) return hr; //2
		if (FAILED(hr = CreateRenderTarget())) return hr;
		if (FAILED(hr = CreateRootSignature())) return hr;
		if (FAILED(hr = InitializeShader("Shaders/simple_vs.cso", "Shaders/simple_ps.cso"))) return hr; //5
		if (FAILED(hr = InitializeBuffers())) return hr;
		return hr;
	}
};