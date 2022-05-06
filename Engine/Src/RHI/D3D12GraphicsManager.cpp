#include "pch.h"
#include "RHI/D3D12GrahpicsManager.h"
#include "Framework/Interface/IApplication.h"

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
			vertex_data_debug_ = new Vector3f[2048];
			color_data_debug_ = new Vector3f[2048];
			result = static_cast<int>(CreateGraphicsResources());
#ifdef _DEBUG
			InitializeBufferDebug();
#endif
		}
		return result;
	}
	void D3d12GraphicsManager::Finalize()
	{
		WaitForPreviousFrame();
		GraphicsManager::Finalize();
		delete[] color_data_debug_;
		delete[] vertex_data_debug_;
	}

	void D3d12GraphicsManager::Present()
	{
		HRESULT hr;
		// swap the back buffer and the front buffer
		hr = p_swapchain->Present(1, 0);
		WaitForPreviousFrame();
	}

	void D3d12GraphicsManager::Draw()
	{
		GraphicsManager::Draw();
		//clear debug vertex data
		//ClearVertexData();
	}
	void D3d12GraphicsManager::UseShaderProgram(const INT32 shaderProgram)
	{

	}
	void D3d12GraphicsManager::SetPerFrameConstants(DrawFrameContext& context)
	{
		UINT8* p_head = p_cbv_data_begin_ + frame_index_ * kSizeConstantBufferPerFrame;
		size_t offset = reinterpret_cast<UINT8*>(&context.lights_) - reinterpret_cast<UINT8*>(&context);
		memcpy(p_head, &context, offset);
		p_head += offset;
		for(int i = 0; i < kMaxLightNum; ++i)
		{
			size_t size = ALIGN(sizeof(Light), 16);
			memcpy(p_head, &context.lights_[i], sizeof(Light));
			p_head += size;
		}
	}

	void D3d12GraphicsManager::SetPerBatchConstants(std::vector<std::shared_ptr<DrawBatchContext>>& batches)
	{
		PerBatchConstants pbc{};
		memset(&pbc, 0x00, sizeof(PerBatchConstants));
		for(auto& p_dbc : batches)
		{
			pbc.object_matrix = Transpose(*p_dbc->node->GetCalculatedTransform());
			pbc.normal_matrix = MatrixInversetranspose(pbc.object_matrix);
			auto mat = p_dbc->material;
			if (mat)
			{
				Color color = mat->GetBaseColor();
				if (color.value_map_)
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
			else
			{
				pbc.base_color = Vector4f(1.f);
				pbc.use_texture = 0.f;
				pbc.specular_color = pbc.base_color;
			}
			uint32_t offset = frame_index_ * kSizeConstantBufferPerFrame + kSizePerFrameConstantBuffer + p_dbc->batch_index
				* kSizePerBatchConstantBuffer;
			memcpy(p_cbv_data_begin_ + offset, &pbc, sizeof(PerBatchConstants));
		}
	}

	void D3d12GraphicsManager::DrawBatch(const std::vector<std::shared_ptr<DrawBatchContext>>& batches)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE cbv_handle[2];
		uint32_t frame_res_desc_offset = frame_index_ * (1 + kMaxSceneObjectCount);

		//offset to batch's pos in constant buf
		cbv_handle[0].ptr = p_cbv_heap_->GetGPUDescriptorHandleForHeapStart().ptr + frame_res_desc_offset * cbv_srv_uav_desc_size_;
		for(const auto& p_dbc : batches)
		{
			const D3dDrawBatchContext& dbc = dynamic_cast<D3dDrawBatchContext&>(*p_dbc);
			cbv_handle[1].ptr = cbv_handle[0].ptr + cbv_srv_uav_desc_size_ * (dbc.batch_index + 1);
			p_cmdlist_->SetGraphicsRootDescriptorTable(1, cbv_handle[1]);
			p_cmdlist_->IASetVertexBuffers(0, dbc.vertex_buf_len, &vertex_buf_view_[dbc.vertex_buf_start]);
			//bind texture
			if (dbc.material)
			{
				if (auto texture = dbc.material->GetBaseColor().value_map_)
				{
					auto texture_index = texture_index_[texture->GetName()];
					D3D12_GPU_DESCRIPTOR_HANDLE srvHandle;
					srvHandle.ptr = p_cbv_heap_->GetGPUDescriptorHandleForHeapStart().ptr + (kTextureDescStartIndex + texture_index) * cbv_srv_uav_desc_size_;
					p_cmdlist_->SetGraphicsRootDescriptorTable(2, srvHandle);
				}
			}
			p_cmdlist_->DrawInstanced(dbc.count, 1, 0, 0);
		}
	}

	void D3d12GraphicsManager::DrawBatch(std::shared_ptr<DrawBatchContext> batch)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE cbv_handle[2];
		uint32_t frame_res_desc_offset = frame_index_ * (1 + kMaxSceneObjectCount);
		//offset to batch's pos in constant buf
		cbv_handle[0].ptr = p_cbv_heap_->GetGPUDescriptorHandleForHeapStart().ptr + frame_res_desc_offset * cbv_srv_uav_desc_size_;
		const D3dDrawBatchContext& dbc = dynamic_cast<D3dDrawBatchContext&>(*batch);
		cbv_handle[1].ptr = cbv_handle[0].ptr + cbv_srv_uav_desc_size_ * (dbc.batch_index + 1);
		p_cmdlist_->SetGraphicsRootDescriptorTable(1, cbv_handle[1]);
		p_cmdlist_->IASetVertexBuffers(0, dbc.vertex_buf_len, &vertex_buf_view_[dbc.vertex_buf_start]);
		//bind texture
		if (dbc.material)
		{
			if (auto texture = dbc.material->GetBaseColor().value_map_)
			{
				auto texture_index = texture_index_[texture->GetName()];
				D3D12_GPU_DESCRIPTOR_HANDLE srvHandle;
				srvHandle.ptr = p_cbv_heap_->GetGPUDescriptorHandleForHeapStart().ptr + (kTextureDescStartIndex + texture_index) * cbv_srv_uav_desc_size_;
				p_cmdlist_->SetGraphicsRootDescriptorTable(2, srvHandle);
			}
		}
		p_cmdlist_->DrawInstanced(dbc.count, 1, 0, 0);
	}

#ifdef _DEBUG
	void D3d12GraphicsManager::DrawLine(const Vector3f& from, const Vector3f& to, const Vector3f& color)
	{
		vertex_data_debug_[cur_debug_vertex_pos] = from;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		vertex_data_debug_[cur_debug_vertex_pos] = to;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		memcpy(p_vex_data_begin, vertex_data_debug_, cur_debug_vertex_pos * 12);
		memcpy(p_color_data_begin, color_data_debug_, cur_debug_vertex_pos * 12);
		draw_batch_contexts_debug_[0].count += 2;
	}
	void D3d12GraphicsManager::DrawBox(const Vector3f& bbMin, const Vector3f& bbMax, const Vector3f& color)
	{
		const auto pre_offset = cur_debug_vertex_pos;
		//bottom
		vertex_data_debug_[cur_debug_vertex_pos] = bbMin;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		vertex_data_debug_[cur_debug_vertex_pos] = bbMin;
		vertex_data_debug_[cur_debug_vertex_pos].z = bbMax.z;
		color_data_debug_[cur_debug_vertex_pos++] = color;

		vertex_data_debug_[cur_debug_vertex_pos] = bbMin;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		vertex_data_debug_[cur_debug_vertex_pos] = bbMin;
		vertex_data_debug_[cur_debug_vertex_pos].x = bbMax.x;
		color_data_debug_[cur_debug_vertex_pos++] = color;

		vertex_data_debug_[cur_debug_vertex_pos] = bbMin;
		vertex_data_debug_[cur_debug_vertex_pos].x = bbMax.x;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		vertex_data_debug_[cur_debug_vertex_pos] = bbMax;
		vertex_data_debug_[cur_debug_vertex_pos].y = bbMin.y;
		color_data_debug_[cur_debug_vertex_pos++] = color;

		vertex_data_debug_[cur_debug_vertex_pos] = bbMax;
		vertex_data_debug_[cur_debug_vertex_pos].y = bbMin.y;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		vertex_data_debug_[cur_debug_vertex_pos] = bbMin;
		vertex_data_debug_[cur_debug_vertex_pos].z = bbMax.z;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		//top
		vertex_data_debug_[cur_debug_vertex_pos] = bbMax;
		vertex_data_debug_[cur_debug_vertex_pos].x = bbMin.x;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		vertex_data_debug_[cur_debug_vertex_pos] = bbMin;
		vertex_data_debug_[cur_debug_vertex_pos].y = bbMax.y;
		color_data_debug_[cur_debug_vertex_pos++] = color;

		vertex_data_debug_[cur_debug_vertex_pos] = bbMin;
		vertex_data_debug_[cur_debug_vertex_pos].y = bbMax.y;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		vertex_data_debug_[cur_debug_vertex_pos] = bbMax;
		vertex_data_debug_[cur_debug_vertex_pos].z = bbMin.z;
		color_data_debug_[cur_debug_vertex_pos++] = color;

		vertex_data_debug_[cur_debug_vertex_pos] = bbMax;
		vertex_data_debug_[cur_debug_vertex_pos].z = bbMin.z;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		vertex_data_debug_[cur_debug_vertex_pos] = bbMax;
		color_data_debug_[cur_debug_vertex_pos++] = color;

		vertex_data_debug_[cur_debug_vertex_pos] = bbMax;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		vertex_data_debug_[cur_debug_vertex_pos] = bbMax;
		vertex_data_debug_[cur_debug_vertex_pos].x = bbMin.x;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		//side
		vertex_data_debug_[cur_debug_vertex_pos] = bbMin;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		vertex_data_debug_[cur_debug_vertex_pos] = bbMin;
		vertex_data_debug_[cur_debug_vertex_pos].y = bbMax.y;
		color_data_debug_[cur_debug_vertex_pos++] = color;

		vertex_data_debug_[cur_debug_vertex_pos] = bbMin;
		vertex_data_debug_[cur_debug_vertex_pos].x = bbMax.x;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		vertex_data_debug_[cur_debug_vertex_pos] = bbMin;
		vertex_data_debug_[cur_debug_vertex_pos].xy = bbMax.xy;
		color_data_debug_[cur_debug_vertex_pos++] = color;

		vertex_data_debug_[cur_debug_vertex_pos] = bbMax;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		vertex_data_debug_[cur_debug_vertex_pos] = bbMax;
		vertex_data_debug_[cur_debug_vertex_pos].y = bbMin.y;
		color_data_debug_[cur_debug_vertex_pos++] = color;

		vertex_data_debug_[cur_debug_vertex_pos] = bbMin;
		vertex_data_debug_[cur_debug_vertex_pos].z = bbMax.z;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		vertex_data_debug_[cur_debug_vertex_pos] = bbMax;
		vertex_data_debug_[cur_debug_vertex_pos].x = bbMin.x;
		color_data_debug_[cur_debug_vertex_pos++] = color;
		memcpy(p_vex_data_begin + pre_offset * 12, &vertex_data_debug_[pre_offset], 288);
		memcpy(p_color_data_begin + pre_offset * 12, &color_data_debug_[pre_offset], 288);
		draw_batch_contexts_debug_[0].count += 24;
	}

	void D3d12GraphicsManager::DrawOverlay()
	{
		auto texture_index = texture_index_["shadow_map"];
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandle;
		srvHandle.ptr = p_cbv_heap_->GetGPUDescriptorHandleForHeapStart().ptr + (kTextureDescStartIndex + texture_index) * cbv_srv_uav_desc_size_;
		p_cmdlist_->SetGraphicsRootDescriptorTable(2, srvHandle);
		p_cmdlist_->IASetVertexBuffers(0, 2, &vertex_buf_view_hud_[0]);
		p_cmdlist_->SetPipelineState(p_plstate_hud_.Get());
		p_cmdlist_->DrawInstanced(6,1,0,0);
	}

	void D3d12GraphicsManager::ClearDebugBuffers()
	{
		vertex_buf_view_debug_.clear();
		draw_batch_contexts_debug_.clear();
		for(auto& buf : buffers_debug_)
			buf.Reset();
		cur_debug_vertex_pos = 0;
	}

	void D3d12GraphicsManager::ClearVertexData()
	{
		if(draw_batch_contexts_debug_.size() > 0)
		{
			memset(p_vex_data_begin, 0x00, 2048);
			draw_batch_contexts_debug_[0].count = 0;
			cur_debug_vertex_pos = 0;
		}
	}

	void D3d12GraphicsManager::InitializeBufferDebug()
	{
		D3D12_HEAP_PROPERTIES prop = { D3D12_HEAP_TYPE_UPLOAD,D3D12_CPU_PAGE_PROPERTY_UNKNOWN,D3D12_MEMORY_POOL_UNKNOWN,1,1 };
		const size_t cb_size = sizeof(Vector3f) * 2048;
		auto buf_desc = CD3DX12_RESOURCE_DESC::Buffer(cb_size);
		ComPtr<ID3D12Resource> vertex_buf;
		p_device_->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &buf_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,IID_PPV_ARGS(&vertex_buf));
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		vertexBufferView.BufferLocation = vertex_buf->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = 12;
		vertexBufferView.SizeInBytes = cb_size;
		vertex_buf_view_debug_.emplace_back(std::move(vertexBufferView));
		D3D12_RANGE readRange = { 0, 0 };
		vertex_buf->Map(0, &readRange, reinterpret_cast<void**>(&p_vex_data_begin));

		ComPtr<ID3D12Resource> color_buf;
		p_device_->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &buf_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&color_buf));
		D3D12_VERTEX_BUFFER_VIEW colorBufferView;
		colorBufferView.BufferLocation = color_buf->GetGPUVirtualAddress();
		colorBufferView.StrideInBytes = 12;
		colorBufferView.SizeInBytes = cb_size;
		vertex_buf_view_debug_.emplace_back(std::move(colorBufferView));
		color_buf->Map(0,&readRange, reinterpret_cast<void**>(&p_color_data_begin));
		buffers_debug_.emplace_back(std::move(vertex_buf));
		buffers_debug_.emplace_back(std::move(color_buf));
		DrawDebugBatchContext dbc{};
		dbc.count = 0;
		draw_batch_contexts_debug_.push_back(std::move(dbc));	
		//-1~1
		Vector3f pos_screen[6]{ {0.5f, 1.f, 0.5f},{1.f, 1.f, 0.5f},{0.5f, 0.5f, 0.5f},{1.f, 1.f, 0.5f},{1.f, 0.5f, 0.5f}
,{0.5f, 0.5f, 0.5f} };
		SceneObjectVertexArray pos_arr(EVertexArrayType::kNormal,0,EVertexDataType::kVertexDataFloat3,&pos_screen,6);
		CreateVertexBuffer(pos_arr,EBufferType::kHUD);
		Vector2f uv_screen[6]{{0.f,0.f},{1.f,0.f},{0.f,1.f},{1.f,0.f},{1.f,1.f},{0.f,1.f}};
		SceneObjectVertexArray uv_arr(EVertexArrayType::kUVs, 0, EVertexDataType::kVertexDataFloat2, &uv_screen, 6);
		CreateVertexBuffer(uv_arr, EBufferType::kHUD);
	}

	void D3d12GraphicsManager::InitializeShaderDebug()
	{
		const char* vs_filename = "Shader/vs_debug.cso";
		const char* fs_filename = "Shader/ps_debug.cso";
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
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
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
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		p_device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&p_plstate_debug_));

		//for hud
		{
			const char* vs_filename = "Shader/vs_hud.cso";
			const char* fs_filename = "Shader/ps_hud.cso";
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
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};
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
			p_device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&p_plstate_hud_));
		}
	}
#endif

	void D3d12GraphicsManager::BeginScene(const Scene& scene)
	{
		GraphicsManager::BeginScene(scene);
		for (auto& _it : scene.Materials)
		{
			auto material = _it.second;
			if (material)
			{
				auto color = material->GetBaseColor();
				if (auto& texture = color.value_map_)
					ThrowIfFailed(CreateTextureBuffer(*texture));
			}
		}
		NE_LOG(ALL,kNormal,"Create draw batch context...")
		int draw_batch_index = 0;
		for (auto& it : scene.GeometryNodes)
		{
			auto pGeometryNode = it.second;
			if (pGeometryNode->Visible())
			{
				auto pGeometry = scene.GetGeometry(pGeometryNode->GetSceneObjectRef());
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
					if (v_property_array.GetType() == EVertexArrayType::kVertex)
						vertex_count = v_property_array.GetVertexCount();
					CreateVertexBuffer(v_property_array);
				}
				if (vertex_count != 0)
				{
					auto dbc = std::make_shared<D3dDrawBatchContext>();
					dbc->count = vertex_count;
					dbc->node = pGeometryNode;
					dbc->batch_index = draw_batch_index;
					dbc->vertex_buf_len = vertexPropertiesCount;
					dbc->vertex_buf_start = vertex_buf_view_.size() - vertexPropertiesCount;
					//TODO:vertex has multi material slot
					auto mat = scene.GetMaterial(pGeometryNode->GetMaterialRef(0));
					if (mat)	dbc->material = mat;
					frames_[frame_index_].batch_contexts.emplace_back(dbc);
					++draw_batch_index;
				}
			}
		}
		NE_LOG(ALL, kNormal, "Done")
		if (SUCCEEDED(p_cmdlist_->Close()))
		{
			ID3D12CommandList* ppCommandLists[] = { p_cmdlist_.Get() };
			p_cmdqueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
			ThrowIfFailed(p_device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(p_fence_.GetAddressOf())));
			fence_value_ = 1;
			fence_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
			if (fence_event_ == NULL)
			{
				ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
			}
			WaitForPreviousFrame();
		}
	}

	void D3d12GraphicsManager::EndScene()
	{
		p_fence_.Reset();
		for (auto p : buffers_)
			p.Reset();
		buffers_.clear();
		for (auto p : textures_)
			p.Reset();
		textures_.clear();
		vertex_buf_view_.clear();
		index_buf_view_.clear();
		textures_.clear();
		texture_index_.clear();
		for (int i = 0; i < 2; ++i)
		{
			auto& batch_context = frames_[i].batch_contexts;
			batch_context.clear();
		}
	}

	void D3d12GraphicsManager::BeginFrame()
	{
		ResetCommandList();
		// Indicate that the back buffer will be used as a render target.
		auto barrier_back_buffer = CD3DX12_RESOURCE_BARRIER::Transition(render_target_arr_[frame_index_].Get(),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		p_cmdlist_->ResourceBarrier(1, &barrier_back_buffer);
		//TODO:Using the depth stencil buffer
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(p_rtv_heap_->GetCPUDescriptorHandleForHeapStart(), frame_index_, rtv_desc_size_);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(p_dsv_heap_->GetCPUDescriptorHandleForHeapStart());
		p_cmdlist_->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		// clear the back buffer to a deep blue
		p_cmdlist_->ClearRenderTargetView(rtvHandle, kBackColor, 0, nullptr);
		p_cmdlist_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		//Set necessary state
		p_cmdlist_->SetGraphicsRootSignature(p_rootsig_.Get());
		ID3D12DescriptorHeap* ppHeaps[] = { p_cbv_heap_.Get(),p_sampler_heap_.Get() };
		p_cmdlist_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		p_cmdlist_->SetGraphicsRootDescriptorTable(3, p_sampler_heap_->GetGPUDescriptorHandleForHeapStart());
		p_cmdlist_->SetPipelineState(p_plstate_.Get());
		p_cmdlist_->RSSetViewports(1, &vp_);
		p_cmdlist_->RSSetScissorRects(1, &rect_);
		p_cmdlist_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//set cbv for per frame
		D3D12_GPU_DESCRIPTOR_HANDLE cbv_handle;
		uint32_t frame_res_desc_offset = frame_index_ * (1 + kMaxSceneObjectCount);
		cbv_handle.ptr = p_cbv_heap_->GetGPUDescriptorHandleForHeapStart().ptr + frame_res_desc_offset * cbv_srv_uav_desc_size_;
		p_cmdlist_->SetGraphicsRootDescriptorTable(0, cbv_handle);
	}

	void D3d12GraphicsManager::EndFrame()
	{
		auto res_barrier = CD3DX12_RESOURCE_BARRIER::Transition(render_target_arr_[frame_index_].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT);
		p_cmdlist_->ResourceBarrier(1, &res_barrier);
		p_cmdlist_->Close();
		// execute the command list
		ID3D12CommandList* ppCommandLists[] = { p_cmdlist_.Get() };
		p_cmdqueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}
	
	void D3d12GraphicsManager::GenerateShadowMapArray(UINT32 count)
	{
		INT32 dsv_offset = 1;
		shadow_map_buf_start_ = textures_.size();
		for(int i = 0; i < count; ++i)
		{
			ComPtr<ID3D12Resource> p_shadow_map = nullptr;
			D3D12_RESOURCE_DESC textureDesc = {};
			textureDesc.MipLevels = 1;
			if(i < count - kMaxPointLightNum)
			{
				textureDesc.Width = g_pApp->GetConfiguration().viewport_width_;
				textureDesc.Height = g_pApp->GetConfiguration().viewport_height_;
				textureDesc.DepthOrArraySize = 1;
			}
			else
			{
				textureDesc.Width = kDefalutCubeShadowMapSize;
				textureDesc.Height = kDefalutCubeShadowMapSize;
				textureDesc.DepthOrArraySize = 6;
			}
			textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			D3D12_CLEAR_VALUE optClear;
			optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;
			CD3DX12_HEAP_PROPERTIES heap_prop(D3D12_HEAP_TYPE_DEFAULT);
			ThrowIfFailed(p_device_->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
				&optClear, IID_PPV_ARGS(p_shadow_map.GetAddressOf())));

			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Texture2D.PlaneSlice = 0;

			if (i < count - kMaxPointLightNum)
			{
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
				dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
				dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				dsvDesc.Texture2D.MipSlice = 0;
				D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
				dsvHandle.ptr = p_dsv_heap_->GetCPUDescriptorHandleForHeapStart().ptr + dsv_desc_size_ * (dsv_offset++);
				p_device_->CreateDepthStencilView(p_shadow_map.Get(), &dsvDesc, dsvHandle);
			}
			else
			{
				for(int j = 0; j < 6; ++j)
				{
					D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
					dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
					dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
					dsvDesc.Texture2DArray.MipSlice = 0;
					dsvDesc.Texture2DArray.FirstArraySlice = j;
					dsvDesc.Texture2DArray.ArraySize = 1;
					D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
					dsvHandle.ptr = p_dsv_heap_->GetCPUDescriptorHandleForHeapStart().ptr + dsv_desc_size_ * (dsv_offset++);
					p_device_->CreateDepthStencilView(p_shadow_map.Get(), &dsvDesc, dsvHandle);
				}
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			}
			D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
			int32_t texture_id = textures_.size();
			srvHandle.ptr = p_cbv_heap_->GetCPUDescriptorHandleForHeapStart().ptr + (kTextureDescStartIndex + texture_id) * cbv_srv_uav_desc_size_;
			p_device_->CreateShaderResourceView(p_shadow_map.Get(), &srvDesc, srvHandle);
			textures_.push_back(p_shadow_map);
		}
		cube_shadow_map_srv_start_ = textures_.size() - kMaxPointLightNum;
		// load the shaders
		const char* vs_filename = "Shader/vs_sm.cso";
		const char* fs_filename = "Shader/ps_sm.cso";
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
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.DepthStencilState = dsd;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 0;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
		psoDesc.SampleDesc.Count = 1;
		p_device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&p_plstate_sm_));
	}

	void D3d12GraphicsManager::BeginShadowMap(Light& light, int light_id,int point_light_id,int cube_map_id) //
	{
		GraphicsManager::BeginShadowMap(light, light_id);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(p_dsv_heap_->GetCPUDescriptorHandleForHeapStart());
		int buf_upload[2]{};
		if (light.type == 1)
		{
			if(cube_map_id == 0)
			{
				D3D12_VIEWPORT vp{ 0.f,0.f,kDefalutCubeShadowMapSize,kDefalutCubeShadowMapSize,0.f,1.f };
				D3D12_RECT rect{0,0,kDefalutCubeShadowMapSize,kDefalutCubeShadowMapSize };
				p_cmdlist_->RSSetViewports(1, &vp);
				p_cmdlist_->RSSetScissorRects(1,&rect);
			}
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(textures_[cube_shadow_map_srv_start_ + point_light_id].Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			p_cmdlist_->ResourceBarrier(1, &barrier);
			dsvHandle.Offset(dsv_desc_size_ * (kMaxLightNum - kMaxPointLightNum + 1 + cube_map_id + (point_light_id * 6)));
			buf_upload[0] = 100 + cube_map_id + (point_light_id * 6);
		}
		else
		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(textures_[shadow_map_buf_start_ + light_id].Get(),
				D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			p_cmdlist_->ResourceBarrier(1, &barrier);
			dsvHandle.Offset(dsv_desc_size_ * (light_id + 1));
			buf_upload[0] = light_id;
		}
		buf_upload[1] = light_id;
		p_cmdlist_->SetGraphicsRoot32BitConstants(4, 2, reinterpret_cast<const void*>(&buf_upload), 0);
		p_cmdlist_->ClearDepthStencilView(dsvHandle,D3D12_CLEAR_FLAG_DEPTH,1.f,0,0,nullptr);
		p_cmdlist_->OMSetRenderTargets(0,nullptr,false,&dsvHandle);
		p_cmdlist_->SetPipelineState(p_plstate_sm_.Get());
	}

	void D3d12GraphicsManager::EndShadowMap(int light_index, bool is_point_light,int point_light_id)
	{
		if(is_point_light)
		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(textures_[cube_shadow_map_srv_start_ + point_light_id].Get(),
				D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
			p_cmdlist_->ResourceBarrier(1, &barrier);
		}
		else
		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(textures_[shadow_map_buf_start_ + light_index].Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
			p_cmdlist_->ResourceBarrier(1, &barrier);
		}
	}

	void D3d12GraphicsManager::EndShadowMap()
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(p_rtv_heap_->GetCPUDescriptorHandleForHeapStart(), frame_index_, rtv_desc_size_);
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(p_dsv_heap_->GetCPUDescriptorHandleForHeapStart());
		p_cmdlist_->RSSetViewports(1, &vp_);
		p_cmdlist_->RSSetScissorRects(1, &rect_);
		p_cmdlist_->SetPipelineState(p_plstate_.Get());
		p_cmdlist_->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
		SetShadowMap();
	}

	void D3d12GraphicsManager::SetShadowMap()
	{
		auto texture_index = shadow_map_buf_start_;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandle;
		srvHandle.ptr = p_cbv_heap_->GetGPUDescriptorHandleForHeapStart().ptr + (kTextureDescStartIndex + texture_index) * cbv_srv_uav_desc_size_;
		p_cmdlist_->SetGraphicsRootDescriptorTable(5, srvHandle);
		srvHandle.ptr = p_cbv_heap_->GetGPUDescriptorHandleForHeapStart().ptr + (kTextureDescStartIndex + cube_shadow_map_srv_start_) * cbv_srv_uav_desc_size_;
		p_cmdlist_->SetGraphicsRootDescriptorTable(6, srvHandle);
	}

	void D3d12GraphicsManager::BeginRenderPass()
	{
		auto& draw_frame_context_ = frames_[frame_index_].frame_context;
		draw_frame_context_.vp_matrix_ = Transpose(p_cam_mgr_->GetCamera().GetView());
		SetPerFrameConstants(draw_frame_context_);
	}

	HRESULT D3d12GraphicsManager::ResetCommandList()
	{
		HRESULT hr = S_OK;
		if (FAILED(hr = p_cmdalloc_->Reset())) return hr;
		if (FAILED(hr = p_cmdlist_->Reset(p_cmdalloc_.Get(), p_plstate_.Get()))) return hr;
		return hr;
	}

	HRESULT D3d12GraphicsManager::CreateCommandList()
	{
		HRESULT hr = S_OK;
		if(p_cmdlist_== nullptr)
		{
			hr = p_device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, p_cmdalloc_.Get(),
				p_plstate_.Get(), IID_PPV_ARGS(&p_cmdlist_));
		}
		return hr;
	}

	HRESULT D3d12GraphicsManager::InitializePSO()
	{
		HRESULT hr = S_OK;
		// load the shaders
		const char* vs_filename = "Shader/vs.cso";
		const char* fs_filename = "Shader/ps.cso";
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
		if (FAILED(hr = p_device_->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&p_plstate_)))) return hr;

		//temp
		InitializeShaderDebug();
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
			assert(kMaxTextureCount > kMaxShadowMapCount);
			dsvHeapDesc.NumDescriptors = kFrameCount + kMaxShadowMapCount;
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			if (FAILED(hr = p_device_->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(p_dsv_heap_.GetAddressOf())))) return hr;
			dsv_desc_size_ = p_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
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
		CD3DX12_ROOT_PARAMETER1 rootParameters[7];
		CD3DX12_DESCRIPTOR_RANGE1 cbv_table[7];
		cbv_table[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,1,0);
		cbv_table[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,1,1);
		cbv_table[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,1,0);
		cbv_table[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER,1,0);
		cbv_table[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV,1,2);
		cbv_table[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,kMaxLightNum - kMaxPointLightNum,1);
		cbv_table[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,kMaxPointLightNum,7);
		rootParameters[0].InitAsDescriptorTable(1, &cbv_table[0]);
		rootParameters[1].InitAsDescriptorTable(1, &cbv_table[1]);
		rootParameters[2].InitAsDescriptorTable(1, &cbv_table[2]);
		rootParameters[3].InitAsDescriptorTable(1, &cbv_table[3]);
		rootParameters[4].InitAsConstants(2,2);
		rootParameters[5].InitAsDescriptorTable(1,&cbv_table[5]);
		rootParameters[6].InitAsDescriptorTable(1,&cbv_table[6]);
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
		frame_index_ = p_swapchain->GetCurrentBackBufferIndex();
		return hr;
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
		frame_index_ = p_swapchain->GetCurrentBackBufferIndex();

		NE_LOG(ALL,kWarning,"CreateDescriptorHeaps...")
		if (FAILED(hr = CreateDescriptorHeaps())) return hr;
		NE_LOG(ALL, kWarning, "Done!")

		NE_LOG(ALL, kWarning, "CreateRenderTarget...")
		if (FAILED(hr = CreateRenderTarget())) return hr;
		NE_LOG(ALL, kWarning, "Done!")

		NE_LOG(ALL, kWarning, "CreateDepthStencil...")
		if (FAILED(hr = CreateDepthStencil())) return hr;
		NE_LOG(ALL, kWarning, "Done!")

		NE_LOG(ALL, kWarning, "CreateRootSignature...")
		if (FAILED(hr = CreateRootSignature())) return hr;
		NE_LOG(ALL, kWarning, "Done!")

		NE_LOG(ALL, kWarning, "InitializePSO...")
		if (FAILED(hr = InitializePSO())) return hr;
		NE_LOG(ALL, kWarning, "Done!")

		NE_LOG(ALL, kWarning, "CreateCommandList...")
		if (FAILED(hr = CreateCommandList())) return hr;
		NE_LOG(ALL, kWarning, "Done!")

		NE_LOG(ALL, kWarning, "CreateConstantBuffer...")
		ThrowIfFailed(hr = CreateConstantBuffer());
		NE_LOG(ALL, kWarning, "Done")

		NE_LOG(ALL, kWarning, "CreateSamplerBuffer...")
		ThrowIfFailed(hr = CreateSamplerBuffer());
		NE_LOG(ALL, kWarning, "Done")

		NE_LOG(ALL, kWarning, "GenerateShadowMapArray...")
		GenerateShadowMapArray();
		NE_LOG(ALL, kWarning, "Done")

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
	HRESULT D3d12GraphicsManager::CreateVertexBuffer(const SceneObjectVertexArray& vertex_array, EBufferType type)
	{
		HRESULT hr = S_OK;
		const uint32_t vertexBufferSize = vertex_array.GetDataSize();
		if(vertexBufferSize == 0)
		{
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
		if(type == EBufferType::kNormal)
		{
			vertex_buf_view_.emplace_back(std::move(vertexBufferView));
			buffers_.emplace_back(std::move(pVertexBufferGPU));
			buffers_.emplace_back(std::move(pVertexBufferUpdate));
		}
		else if(type == EBufferType::kHUD)
		{
			vertex_buf_view_hud_.emplace_back(std::move(vertexBufferView));
			buffers_hud_.emplace_back(std::move(pVertexBufferGPU));
			buffers_hud_.emplace_back(std::move(pVertexBufferUpdate));
		}
		return hr;
	}
};