#include "pch.h"
#include "RHI/D3D12GrahpicsManager.h"
#include "Framework/Interface/IApplication.h"
#include "Framework/Common/Buffer.h"
#include "Framework/Common/AssetLoader.h"
#include "Framework/Common/SceneManager.h"
#include "Framework/Common/Log.h"




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
	static uint64_t CalcConstantBufferByteSize(uint64_t byte_size)
	{
		return (byte_size + 255) & ~255;
	}
}

namespace Engine
{
	int D3d12GraphicsManager::Initialize()
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
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(p_dsv_heap_->GetCPUDescriptorHandleForHeapStart());
		p_cmdlist_->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		// clear the back buffer to a deep blue
		p_cmdlist_->ClearRenderTargetView(rtvHandle, kBackColor, 0, nullptr);
		p_cmdlist_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}
	void D3d12GraphicsManager::Draw()
	{
		PopulateCommandList();
		RenderBuffers();
		WaitForPreviousFrame();
	}
	void D3d12GraphicsManager::Update()
	{

	}
#ifdef _DEBUG
	void D3d12GraphicsManager::DrawLine(const Vector3f& from, const Vector3f& to, const Vector3f& color)
	{
	}
	void D3d12GraphicsManager::DrawBox(const Vector3f& bbMin, const Vector3f& bbMax, const Vector3f& color)
	{
	}
	void D3d12GraphicsManager::ClearDebugBuffers()
	{
	}
#endif
	bool D3d12GraphicsManager::SetPerFrameShaderParameters()
	{
		//temp
		draw_frame_context_.view_matrix_ = Transpose(p_cam_mgr_->GetCamera().GetView());
		draw_frame_context_.projection_matrix_ = Transpose(p_cam_mgr_->GetCamera().GetProjection());
		draw_frame_context_.camera_position_ = Vector4f(p_cam_mgr_->GetCamera().GetPosition(),1.f);
		memcpy(p_cbv_data_begin_ + cur_back_buf_index_ * kSizeConstantBufferPerFrame, &draw_frame_context_, sizeof(draw_frame_context_));
		return true;
	}
	bool D3d12GraphicsManager::SetPerBatchShaderParameters(int32_t index)
	{
		PerBatchConstants pbc{};
		memset(&pbc,0x00,sizeof(PerBatchConstants));
		pbc.object_matrix = Transpose(*draw_batch_contexts_[index].node->GetCalculatedTransform());
		pbc.normal_matrix = MatrixInversetranspose(pbc.object_matrix);
		auto mat = draw_batch_contexts_[index].material;
		if(mat)
		{
			Color color = mat->GetBaseColor();
			if(color.value_map_)
			{
				pbc.base_color = Vector4f(0.f);
				pbc.use_texture = 1.f;
			}
			else 
			{
				pbc.base_color = color.value_;
				pbc.use_texture = 0.f;
			}
			color = mat->GetSpecularColor();
			if (color.value_map_) pbc.specular_color = Vector4f(0.f);
			else pbc.specular_color = color.value_;
			Parameter param = mat->GetSpecularPower();
			pbc.specular_power = param.value_;
		}
		uint32_t offset = cur_back_buf_index_ * kSizeConstantBufferPerFrame + kSizePerFrameConstantBuffer + index * kSizePerFrameConstantBuffer;
		memcpy(p_cbv_data_begin_ + offset,&pbc, sizeof(PerBatchConstants));
		return true;
	}
	HRESULT D3d12GraphicsManager::InitializeBuffers()
	{
		HRESULT hr = S_OK;
		//TODO using kinds of buffer
		if (FAILED(hr = CreateDepthStencil())) return hr;
		if (FAILED(hr = CreateConstantBuffer())) return hr;
		if (FAILED(hr = CreateSamplerBuffer())) return hr;
		auto* scene = g_pSceneManager->GetSceneForRendering();
		for(auto _it : scene->Materials)
		{
			auto material = _it.second;
			if(material)
			{
				auto color = material->GetBaseColor();
				if(auto texture = color.value_map_)
					ThrowIfFailed(hr = CreateTextureBuffer(*texture));
			}
		}
		if (scene != nullptr) 
		{
			auto pGeometryNode = scene->GetFirstGeometryNode();
			int32_t n = 0;
			while (pGeometryNode)
			{
				if (pGeometryNode->Visible()) 
				{
					auto pGeometry = scene->GetGeometry(pGeometryNode->GetSceneObjectRef());
					assert(pGeometry);
					auto pMesh = pGeometry->GetMesh().lock();
					if (!pMesh) continue;
					// Set the number of vertex properties.
					auto vertexPropertiesCount = pMesh->GetVertexPropertiesCount();
					// Set the number of vertices in the vertex array.
					size_t vertex_count = 0;
					for (decltype(vertexPropertiesCount) i = 0; i < vertexPropertiesCount; i++) 
					{
						const SceneObjectVertexArray& v_property_array = pMesh->GetVertexPropertyArray(i);
						if(v_property_array.GetType() == EVertexArrayType::kVertex)
							vertex_count = v_property_array.GetVertexCount();
						CreateVertexBuffer(v_property_array);
					}
					if (vertex_count != 0)
					{
						DrawBatchContext dbc{};
						dbc.count = vertex_count;
						dbc.node = pGeometryNode;
						//TODO:vertex has multi material slot
						auto mat = scene->GetMaterial(pGeometryNode->GetMaterialRef(0));
						if(mat)	dbc.material = mat;
						draw_batch_contexts_.push_back(std::move(dbc));
						draw_batch_contexts_.back().vertex_buf_len_ = vertexPropertiesCount;
						draw_batch_contexts_.back().vertex_buf_start_ = vertex_buf_view_.size() - vertexPropertiesCount;
						++n;
					}
				}
				pGeometryNode = scene->GetNextGeometryNode();
			}
		}
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
		{
			auto aspect = 16.f / 9.f;
			Matrix4x4f view{};
			Matrix4x4f projection{};	
			//lenth is cm			
			p_cam_mgr_ = std::make_unique<CameraManager>();
			BuildViewMatrixLookAtLH(view, Vector3f{ 0.f, 0.0f, -1000.f }, Vector3f{ 0.0f, 0.0f, 1000.f }, Vector3f{ 0.0f, 1.0f, 0.0f });
			BuildPerspectiveFovLHMatrix(projection, 1.57F, aspect, 100.f, 100000.f);
			draw_frame_context_.view_matrix_ = Transpose(p_cam_mgr_->GetCamera().GetView());
			draw_frame_context_.projection_matrix_ = Transpose(p_cam_mgr_->GetCamera().GetProjection());
			draw_frame_context_.world_matrix_ = BuildIdentityMatrix();
			draw_frame_context_.ambient_color_ = Vector4f{0.1f,0.1f,0.1f,0.f};
			draw_frame_context_.light_color_ = Vector4f{1.f,1.f,1.f,1.f};
			SetPerFrameShaderParameters();
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
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
		vertex_buf_per_frame_num_ = _countof(ied);
		//TODO: Create D3D12_RASTERIZER_DESC D3D12_RENDER_TARGET_BLEND_DESC D3D12_BLEND_DESC D3D12_DEPTH_STENCILOP_DESC D3D12_DEPTH_STENCIL_DESC
		//Create DepthSenticil Desc
		const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = { D3D12_STENCIL_OP_KEEP,D3D12_STENCIL_OP_KEEP,D3D12_STENCIL_OP_KEEP,D3D12_COMPARISON_FUNC_ALWAYS };
		D3D12_DEPTH_STENCIL_DESC dsd = { TRUE,D3D12_DEPTH_WRITE_MASK_ALL,D3D12_COMPARISON_FUNC_LESS,FALSE,D3D12_DEFAULT_STENCIL_READ_MASK,
			D3D12_DEFAULT_STENCIL_WRITE_MASK,defaultStencilOp, defaultStencilOp };
		//Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { ied, _countof(ied) };
		psoDesc.pRootSignature = p_rootsig_.Get();
		psoDesc.VS = vertexShaderByteCode;
		psoDesc.PS = pixelShaderByteCode;
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.DepthStencilState = dsd;
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
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = kFrameCount;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			if (FAILED(hr = p_device_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(p_rtv_heap_.GetAddressOf())))) return hr;
			rtv_desc_size_ = p_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}
		// Describeand create a depth stencil view(DSV) descriptor heap.
		{
			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
			dsvHeapDesc.NumDescriptors = 1;
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			if (FAILED(hr = p_device_->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(p_dsv_heap_.GetAddressOf())))) return hr;
		}
		// Describe and create a Shader Resource View (SRV) and 
		// Constant Buffer View (CBV) and 
		// Unordered Access View (UAV) descriptor heap.
		{
			D3D12_DESCRIPTOR_HEAP_DESC srv_cbv_uavHeapDesc = {};
			srv_cbv_uavHeapDesc.NumDescriptors = kFrameCount * (1 + kMaxSceneObjectCount) + kMaxTextureCount;
			srv_cbv_uavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srv_cbv_uavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			if (FAILED(hr = p_device_->CreateDescriptorHeap(&srv_cbv_uavHeapDesc, IID_PPV_ARGS(p_cbv_heap_.GetAddressOf())))) return hr;
			cbv_srv_uav_desc_size_ = p_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		{
			D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
			samplerHeapDesc.NumDescriptors = kMaxTextureCount; // this is the max D3d12 HW support currently
			samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			if (FAILED(hr = p_device_->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(p_sampler_heap_.GetAddressOf())))) return hr;
		}
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
	HRESULT D3d12GraphicsManager::CreateDepthStencil()
	{
		HRESULT hr;
		D3D12_DEPTH_STENCIL_VIEW_DESC ds_desc = {};
		ds_desc.Format = DXGI_FORMAT_D32_FLOAT;
		ds_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		ds_desc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE clear_val = {};
		clear_val.Format = DXGI_FORMAT_D32_FLOAT;
		clear_val.DepthStencil.Depth = 1.f;
		clear_val.DepthStencil.Stencil = 0u;

		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		uint32_t width = g_pApp->GetConfiguration().viewport_width_;
		uint32_t height = g_pApp->GetConfiguration().viewport_height_;
		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = width;
		resourceDesc.Height = height;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 0;
		resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		if (FAILED(hr = p_device_->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &resourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_val, IID_PPV_ARGS(p_ds_buffer_.GetAddressOf()))))
			return hr;
		p_device_->CreateDepthStencilView(p_ds_buffer_.Get(), &ds_desc, p_dsv_heap_->GetCPUDescriptorHandleForHeapStart());
		return hr;
	}
	HRESULT D3d12GraphicsManager::CreateRootSignature()
	{
		HRESULT hr = S_OK;
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(p_device_->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}
		CD3DX12_ROOT_PARAMETER1 rootParameters[4];
		CD3DX12_DESCRIPTOR_RANGE1 cbv_table[4];
		cbv_table[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,1,0);
		cbv_table[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,1,1);
		cbv_table[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,1,0);
		cbv_table[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,1,0);
		rootParameters[0].InitAsDescriptorTable(1, &cbv_table[0]);
		rootParameters[1].InitAsDescriptorTable(1, &cbv_table[1]);
		rootParameters[2].InitAsDescriptorTable(1, &cbv_table[2]);
		rootParameters[3].InitAsDescriptorTable(1, &cbv_table[3]);
		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		if (FAILED(hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error))) return hr;
		if (FAILED(hr = p_device_->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&p_rootsig_)))) return hr;
		return hr;
	}
	HRESULT D3d12GraphicsManager::WaitForPreviousFrame()
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
	HRESULT D3d12GraphicsManager::PopulateCommandList()
	{
		HRESULT hr =S_OK;
		p_cmdlist_->SetGraphicsRootSignature(p_rootsig_.Get());
		ID3D12DescriptorHeap* ppHeaps[] = { p_cbv_heap_.Get(),p_sampler_heap_.Get() };
		p_cmdlist_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		 //Get the background buffer per-frame-cbv descriptor handle
		D3D12_GPU_DESCRIPTOR_HANDLE cbv_handle[2];
		uint32_t frame_res_desc_offset = cur_back_buf_index_ * (1 + kMaxSceneObjectCount);
		cbv_handle[0].ptr = p_cbv_heap_->GetGPUDescriptorHandleForHeapStart().ptr + frame_res_desc_offset * cbv_srv_uav_desc_size_;
		SetPerFrameShaderParameters();
		p_cmdlist_->SetGraphicsRootDescriptorTable(0, cbv_handle[0]);
		p_cmdlist_->RSSetViewports(1, &vp_);
		p_cmdlist_->RSSetScissorRects(1, &rect_);
		p_cmdlist_->SetGraphicsRootDescriptorTable(3,p_sampler_heap_->GetGPUDescriptorHandleForHeapStart());
		p_cmdlist_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		for(uint32_t i = 0; i < draw_batch_contexts_.size(); i++) 
		{
			SetPerBatchShaderParameters(i);
			cbv_handle[1].ptr = cbv_handle[0].ptr + cbv_srv_uav_desc_size_ * (i + 1);
			p_cmdlist_->SetGraphicsRootDescriptorTable(1, cbv_handle[1]);
			p_cmdlist_->IASetVertexBuffers(0, draw_batch_contexts_[i].vertex_buf_len_, &vertex_buf_view_[draw_batch_contexts_[i].vertex_buf_start_]);
			//p_cmdlist_->IASetVertexBuffers(0, 3, &vertex_buf_view_[i * vertex_buf_per_frame_num_]);
			//bind texture
			if(draw_batch_contexts_[i].material)
			{
				if(auto texture = draw_batch_contexts_[i].material->GetBaseColor().value_map_)
				{
					auto texture_index = texture_index_[texture->GetName()];
					D3D12_GPU_DESCRIPTOR_HANDLE srvHandle;
					srvHandle.ptr = p_cbv_heap_->GetGPUDescriptorHandleForHeapStart().ptr + (kTextureDescStartIndex + texture_index) * cbv_srv_uav_desc_size_;
					p_cmdlist_->SetGraphicsRootDescriptorTable(2, srvHandle);
				}
			}
			p_cmdlist_->DrawInstanced(draw_batch_contexts_[i].count,1,0,0);
		}
		auto res_barrier = CD3DX12_RESOURCE_BARRIER::Transition(render_target_arr_[cur_back_buf_index_].Get(),D3D12_RESOURCE_STATE_RENDER_TARGET, 
			D3D12_RESOURCE_STATE_PRESENT);
		p_cmdlist_->ResourceBarrier(1, &res_barrier);
		return hr = p_cmdlist_->Close();
	}
	HRESULT D3d12GraphicsManager::CreateGraphicsResources()
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
		//if (FAILED(hr = InitializeShader("Shader/simple_vs.cso", "Shader/simple_ps.cso"))) return hr; //5
		if (FAILED(hr = InitializeShader("Shader/vs.cso", "Shader/ps.cso"))) return hr; //5
		if (FAILED(hr = InitializeBuffers())) return hr;
		return hr;
	}
	HRESULT D3d12GraphicsManager::CreateSamplerBuffer()
	{
		// Describe and create a sampler.
		D3D12_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		p_device_->CreateSampler(&samplerDesc, p_sampler_heap_->GetCPUDescriptorHandleForHeapStart());
		return S_OK;
	}
	HRESULT D3d12GraphicsManager::CreateTextureBuffer(SceneObjectTexture& texture)
	{
		HRESULT hr = S_OK;
		auto it = texture_index_.find(texture.GetName());
		if(it == texture_index_.end())
		{
			auto image = texture.GetImage();
			// Describe and create a Texture2D.
			ComPtr<ID3D12Resource> pTextureGPU;
			ComPtr<ID3D12Resource> pTextureUpload;
			D3D12_RESOURCE_DESC textureDesc = {};
			textureDesc.MipLevels = 1;
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			textureDesc.Width = image.width;
			textureDesc.Height = image.height;
			textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			textureDesc.DepthOrArraySize = 1;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			CD3DX12_HEAP_PROPERTIES heap_prop(D3D12_HEAP_TYPE_DEFAULT);
			ThrowIfFailed(hr = p_device_->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr, IID_PPV_ARGS(pTextureGPU.GetAddressOf())));
			const UINT subresourceCount = textureDesc.DepthOrArraySize * textureDesc.MipLevels;
			const UINT64 uploadBufferSize = GetRequiredIntermediateSize(pTextureGPU.Get(), 0, subresourceCount);
			heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
			auto upload_buf_desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
			hr = p_device_->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &upload_buf_desc, D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr, IID_PPV_ARGS(pTextureUpload.GetAddressOf()));
			//ThrowIfFailed();
			D3D12_SUBRESOURCE_DATA textureData = {};
			textureData.pData = image.data;
			textureData.RowPitch = image.pitch;
			textureData.SlicePitch = image.pitch * image.height * 4;
			UpdateSubresources(p_cmdlist_.Get(), pTextureGPU.Get(), pTextureUpload.Get(), 0, 0, subresourceCount, &textureData);
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = pTextureGPU.Get();
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			p_cmdlist_->ResourceBarrier(1, &barrier);
			// Describe and create a SRV for the texture.
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = textureDesc.Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = -1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
			int32_t texture_id = texture_index_.size();
			srvHandle.ptr = p_cbv_heap_->GetCPUDescriptorHandleForHeapStart().ptr + (kTextureDescStartIndex + texture_id) * cbv_srv_uav_desc_size_;
			p_device_->CreateShaderResourceView(pTextureGPU.Get(), &srvDesc, srvHandle);
			texture_index_[texture.GetName()] = texture_id;
			buffers_.push_back(pTextureUpload.Get());
			textures_.push_back(pTextureGPU.Get());
		}
		else return hr;
	}
	HRESULT D3d12GraphicsManager::CreateConstantBuffer()
	{
		HRESULT hr;
		//Create Buffer for cb
		ComPtr<ID3D12Resource> pConstantUploadBuffer;
		{
			D3D12_HEAP_PROPERTIES prop = { D3D12_HEAP_TYPE_UPLOAD,D3D12_CPU_PAGE_PROPERTY_UNKNOWN,D3D12_MEMORY_POOL_UNKNOWN,1,1 };
			auto cb_size = kSizeConstantBufferPerFrame * kFrameCount; //CalcConstantBufferByteSize(kSizeConstantBufferPerFrame * kFrameCount);
			auto buf_desc = CD3DX12_RESOURCE_DESC::Buffer(cb_size);
			if (hr = p_device_->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &buf_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
				IID_PPV_ARGS(&pConstantUploadBuffer))) 
			return hr;		
		}
		//Create ConstantBufferView
		{
			for (uint32_t i = 0; i < kFrameCount; i++)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle;
				cbvHandle.ptr = p_cbv_heap_->GetCPUDescriptorHandleForHeapStart().ptr + i * (1 + kMaxSceneObjectCount) * cbv_srv_uav_desc_size_;
				// Describe and create a per frame constant buffer view.
				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
				cbvDesc.BufferLocation = pConstantUploadBuffer->GetGPUVirtualAddress();
				cbvDesc.SizeInBytes = kSizePerFrameConstantBuffer;
				p_device_->CreateConstantBufferView(&cbvDesc, cbvHandle);
				for (uint32_t j = 0; j < kMaxSceneObjectCount; j++)
				{
					D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle2;
					cbvHandle2.ptr = cbvHandle.ptr + (j + 1) * cbv_srv_uav_desc_size_;
					// Describe and create a per frame constant buffer view.
					cbvDesc.BufferLocation = pConstantUploadBuffer->GetGPUVirtualAddress() + kSizePerFrameConstantBuffer + j * kSizePerBatchConstantBuffer;
					cbvDesc.SizeInBytes = kSizePerBatchConstantBuffer;
					p_device_->CreateConstantBufferView(&cbvDesc, cbvHandle2);
				}
			}
		}
		D3D12_RANGE readRange = { 0, 0 };
		// Map and initialize the constant buffer. We don't unmap this until the
		// app closes. Keeping things mapped for the lifetime of the resource is okay.
		hr = pConstantUploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&p_cbv_data_begin_));
		buffers_.push_back(pConstantUploadBuffer);
		return hr;
	}
	HRESULT D3d12GraphicsManager::CreateIndexBuffer(const SceneObjectIndexArray& index_array)
	{
		HRESULT hr = S_OK;
		const uint32_t indices_size = index_array.GetDataSize();
		ComPtr<ID3D12Resource> pIndexBufferGPU = nullptr;
		ComPtr<ID3D12Resource> pIndexBufferUpdate = nullptr;
		// create index GPU heap
		auto heap_properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto res_desc = CD3DX12_RESOURCE_DESC::Buffer(indices_size);
		if (FAILED(hr = p_device_->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&pIndexBufferGPU))))
			return hr;
		heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
		if (FAILED(hr = p_device_->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pIndexBufferUpdate))))
			return hr;
		// Copy the indies data to the index buffer.
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = index_array.GetData();
		UpdateSubresources<1>(p_cmdlist_.Get(), pIndexBufferGPU.Get(), pIndexBufferUpdate.Get(), 0, 0, 1, &indexData);
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = pIndexBufferGPU.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		p_cmdlist_->ResourceBarrier(1, &barrier);
		// initialize the index buffer view
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		indexBufferView.BufferLocation = pIndexBufferGPU->GetGPUVirtualAddress();
		indexBufferView.Format = DXGI_FORMAT_R32_UINT;
		indexBufferView.SizeInBytes = indices_size;
		index_buf_view_.push_back(indexBufferView);
		buffers_.push_back(pIndexBufferGPU);
		buffers_.push_back(pIndexBufferUpdate);
		return hr;
	}
	HRESULT D3d12GraphicsManager::CreateVertexBuffer(const SceneObjectVertexArray& vertex_array)
	{
		HRESULT hr = S_OK;
		const uint32_t vertexBufferSize = vertex_array.GetDataSize();
		if(vertexBufferSize == 0)
		{
			D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
			vertexBufferView.BufferLocation = NULL;
			vertexBufferView.StrideInBytes = 0;
			vertexBufferView.SizeInBytes = 0;
			vertex_buf_view_.emplace_back(std::move(vertexBufferView));
			NE_LOG(ALL,kWarning,"some vertex_array has 0 data size")
			return hr;
		}
		D3D12_HEAP_PROPERTIES heap_properties = {};
		heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heap_properties.CreationNodeMask = 1;
		heap_properties.VisibleNodeMask = 1;
		D3D12_RESOURCE_DESC res_desc = {};
		res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		res_desc.Alignment = 0;
		res_desc.Width = vertexBufferSize;
		res_desc.Height = 1;
		res_desc.DepthOrArraySize = 1;
		res_desc.MipLevels = 1;
		res_desc.Format = DXGI_FORMAT_UNKNOWN;
		res_desc.SampleDesc.Count = 1;
		res_desc.SampleDesc.Quality = 0;
		res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		ComPtr<ID3D12Resource> pVertexBufferGPU = nullptr;
		ComPtr<ID3D12Resource> pVertexBufferUpdate = nullptr;
		// create index GPU heap
		if (FAILED(hr = p_device_->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&pVertexBufferGPU))))
			return hr;
		heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
		if (FAILED(hr = p_device_->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE,
			&res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&pVertexBufferUpdate))))
			return hr;
		// Copy the indies data to the vertex buffer.
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = vertex_array.GetData();
		vertexData.RowPitch = vertexBufferSize;
		UpdateSubresources<1>(p_cmdlist_.Get(), pVertexBufferGPU.Get(), pVertexBufferUpdate.Get(), 0, 0, 1, &vertexData);
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = pVertexBufferGPU.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		p_cmdlist_->ResourceBarrier(1, &barrier);
		// Initialize the vertex buffer view.
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		vertexBufferView.BufferLocation = pVertexBufferGPU->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = vertexBufferSize / vertex_array.GetVertexCount();
		vertexBufferView.SizeInBytes = vertexBufferSize;
		vertex_buf_view_.emplace_back(std::move(vertexBufferView));
		buffers_.emplace_back(std::move(pVertexBufferGPU));
		buffers_.emplace_back(std::move(pVertexBufferUpdate));
		return hr;
	}
};