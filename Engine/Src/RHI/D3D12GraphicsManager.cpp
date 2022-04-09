#pragma once
#include "pch.h"
#include "RHI/D3D12GrahpicsManager.h"
#include "Framework/Interface/IApplication.h"
#include "Framework/Common/Buffer.h"
#include "Framework/Common/AssetLoader.h"
#include "Framework/Common/SceneManager.h"

namespace Engine
{
    template<typename T>
    inline void SafeRelease(T** ppT)
    {
        if (*ppT != nullptr)
        {
            (*ppT)->Release();
            (*ppT) = nullptr;
        }
    }

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

    HRESULT D3d12GraphicsManager::CreateIndexBuffer(const SceneObjectIndexArray& index_array)
    {
        HRESULT hr;
        ComPtr<ID3D12Resource> pIndexBufferUploadHeap;
        // create index GPU heap
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_DEFAULT;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;
        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = index_array.GetDataSize();
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        ComPtr<ID3D12Resource> pIndexBuffer;
        if (FAILED(hr = p_device_->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(pIndexBuffer.GetAddressOf())))) return hr;
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        if (FAILED(hr = p_device_->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(pIndexBufferUploadHeap.GetAddressOf())
        ))) return hr;

        D3D12_SUBRESOURCE_DATA indexData = {};
        indexData.pData = index_array.GetData();

        UpdateSubresources<1>(p_cmdlist_.Get(), pIndexBuffer.Get(), pIndexBufferUploadHeap.Get(), 0, 0, 1, &indexData);
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = pIndexBuffer.Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        p_cmdlist_->ResourceBarrier(1, &barrier);

        // initialize the index buffer view
        D3D12_INDEX_BUFFER_VIEW indexBufferView;
        indexBufferView.BufferLocation = pIndexBuffer->GetGPUVirtualAddress();
        indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        indexBufferView.SizeInBytes = (UINT)index_array.GetDataSize();
        index_buf_view_.push_back(indexBufferView);
        buffers_.push_back(pIndexBuffer);
        buffers_.push_back(pIndexBufferUploadHeap);

        DrawBatchContext dbc;
        dbc.count = (UINT)index_array.GetIndexCount();
        draw_batch_context_.push_back(std::move(dbc));
        return hr;
    }

    HRESULT D3d12GraphicsManager::CreateVertexBuffer(const SceneObjectVertexArray& v_property_array)
    {
        HRESULT hr;
        ComPtr<ID3D12Resource> p_vertex_buf_upload_heap;
        // create vertex GPU heap 
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_DEFAULT;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;
        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = v_property_array.GetDataSize();
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        ComPtr<ID3D12Resource> pVertexBuffer;
        if (FAILED(hr = p_device_->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &resourceDesc,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(pVertexBuffer.GetAddressOf()))))
            return hr;
        prop.Type = D3D12_HEAP_TYPE_UPLOAD;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;
        if (FAILED(hr = p_device_->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(p_vertex_buf_upload_heap.GetAddressOf()))))
            return hr;

        D3D12_SUBRESOURCE_DATA vertexData = {};
        vertexData.pData = v_property_array.GetData();
        UpdateSubresources<1>(p_cmdlist_.Get(), pVertexBuffer.Get(), p_vertex_buf_upload_heap.Get(), 0, 0, 1, &vertexData);
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = pVertexBuffer.Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        p_cmdlist_->ResourceBarrier(1, &barrier);
        // initialize the index buffer view
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
        vertexBufferView.BufferLocation = pVertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.StrideInBytes = (UINT)(v_property_array.GetDataSize() / v_property_array.GetVertexCount());
        vertexBufferView.SizeInBytes = (UINT)v_property_array.GetDataSize();
        vertex_buf_view_.push_back(vertexBufferView);
        buffers_.push_back(pVertexBuffer);
        buffers_.push_back(p_vertex_buf_upload_heap);
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
        D3D12_DESCRIPTOR_RANGE1 ranges[3] = {
            { D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC },
            { D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0 },
            { D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0,D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC }
        };
        D3D12_ROOT_PARAMETER1 rootParameters[3] = {
            { D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { 1, &ranges[0] }, D3D12_SHADER_VISIBILITY_ALL },
            { D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { 1, &ranges[1] }, D3D12_SHADER_VISIBILITY_PIXEL },
            { D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, { 1, &ranges[2] }, D3D12_SHADER_VISIBILITY_PIXEL }
        };
        // Allow input layout and deny uneccessary access to certain pipeline stages.
        D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        D3D12_ROOT_SIGNATURE_DESC1 rootSignatureDesc = {
                _countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags
        };

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc = {
            D3D_ROOT_SIGNATURE_VERSION_1_1,
        };

        versionedRootSignatureDesc.Desc_1_1 = rootSignatureDesc;

        ComPtr<ID3DBlob> signature = nullptr;
        ComPtr<ID3DBlob> error = nullptr;
        if (SUCCEEDED(hr = D3D12SerializeVersionedRootSignature(&versionedRootSignatureDesc, signature.GetAddressOf(), error.GetAddressOf())))
        {
            hr = p_device_->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(p_rootsig_.GetAddressOf()));
        }
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

    HRESULT D3d12GraphicsManager::PopulateCommandList()
    {
        HRESULT hr;
        
        // Set necessary state.
        p_cmdlist_->SetGraphicsRootSignature(p_rootsig_.Get());

        ID3D12DescriptorHeap* ppHeaps[] = { p_cbv_heap_.Get(), p_sampler_heap_.Get()};
        p_cmdlist_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

        // CBV Per Frame
        D3D12_GPU_DESCRIPTOR_HANDLE cbvSrvHandle[2];
        uint32_t nFrameResourceDescriptorOffset = frame_index_ * (1 + kMaxSceneObjectCount);
        cbvSrvHandle[0].ptr = p_cbv_heap_->GetGPUDescriptorHandleForHeapStart().ptr + nFrameResourceDescriptorOffset * cbv_srv_uav_desc_size_;

        // Sampler
        p_cmdlist_->SetGraphicsRootDescriptorTable(1, p_sampler_heap_->GetGPUDescriptorHandleForHeapStart());

        // SRV
        D3D12_GPU_DESCRIPTOR_HANDLE srvHandle;
        srvHandle.ptr = p_cbv_heap_->GetGPUDescriptorHandleForHeapStart().ptr + kTextureDescStartIndex * cbv_srv_uav_desc_size_;
        p_cmdlist_->SetGraphicsRootDescriptorTable(2, srvHandle);

        p_cmdlist_->RSSetViewports(1, &vp_);
        p_cmdlist_->RSSetScissorRects(1, &rect_);
        p_cmdlist_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        SetPerFrameShaderParameters();

        // do 3D rendering on the back buffer here
        cbvSrvHandle[1].ptr = p_cbv_heap_->GetGPUDescriptorHandleForHeapStart().ptr + nFrameResourceDescriptorOffset * cbv_srv_uav_desc_size_;
        
        int32_t i = 0;
        for (auto dbc : draw_batch_context_)
        {
            // CBV Per Batch
            cbvSrvHandle[1].ptr = cbv_srv_uav_desc_size_;
            p_cmdlist_->SetGraphicsRootDescriptorTable(0, cbvSrvHandle[0]);

            // select which vertex buffer(s) to use
            p_cmdlist_->IASetVertexBuffers(0, 2, &vertex_buf_view_[0]);
            // select which index buffer to use
            p_cmdlist_->IASetIndexBuffer(&index_buf_view_[i]);

            // draw the vertex buffer to the back buffer
            p_cmdlist_->DrawIndexedInstanced(dbc.count, 1, 0, 0, 0);
            i++;
        }
        
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = p_rt_[frame_index_].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        p_cmdlist_->ResourceBarrier(1, &barrier);

        hr = p_cmdlist_->Close();

        return hr;
    }

    int Engine::D3d12GraphicsManager::Initialize()
    {
        int result = GraphicsManager::Initialize();
        if (!result)
        {
            const GfxConfiguration& config = g_pApp->GetConfiguration();
            vp_ = { 0.0f, 0.0f, static_cast<float>(config.viewport_width_), static_cast<float>(config.viewport_height_), 0.0f, 1.0f };
            rect_ = { 0, 0, static_cast<LONG>(config.viewport_width_), static_cast<LONG>(config.viewport_height_) };
            result = static_cast<int>(CreateGraphicsResources());
        }
        return result;
    }

    void Engine::D3d12GraphicsManager::Finalize()
    {
        WaitForPreviousFrame();
    }

    void Engine::D3d12GraphicsManager::Tick()
    {
    }

    void D3d12GraphicsManager::Clear()
    {
        HRESULT hr;
        // command list allocators can only be reset when the associated 
        // command lists have finished execution on the GPU; apps should use 
        // fences to determine GPU execution progress.
        if (FAILED(hr = p_cmdalloc_->Reset()))
        {
            return;
        }
        // however, when ExecuteCommandList() is called on a particular command 
        // list, that command list can then be reset at any time and must be before 
        // re-recording.
        if (FAILED(hr = p_cmdlist_->Reset(p_cmdalloc_.Get(), p_plstate_.Get())))
        {
            return;
        }

        // Indicate that the back buffer will be used as a render target.
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = p_rt_[frame_index_].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        p_cmdlist_->ResourceBarrier(1, &barrier);

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
        rtvHandle.ptr = p_rtv_heap_->GetCPUDescriptorHandleForHeapStart().ptr + frame_index_ * rtv_desc_size_;
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;
        dsvHandle = p_dsv_heap_->GetCPUDescriptorHandleForHeapStart();
        p_cmdlist_->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

        // clear the back buffer to a deep blue
        const FLOAT clearColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
        p_cmdlist_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        p_cmdlist_->ClearDepthStencilView(p_dsv_heap_->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }

    void D3d12GraphicsManager::Draw()
    {
        PopulateCommandList();
        RenderBuffers();
        WaitForPreviousFrame();
    }

    bool D3d12GraphicsManager::SetPerFrameShaderParameters()
    {
        memcpy(p_cbv_data_begin_ + frame_index_ * kSizeConstantBufferPerFrame, &draw_frame_context_, sizeof(draw_frame_context_));
        return true;
    }

    bool D3d12GraphicsManager::SetPerBatchShaderParameters(int32_t index)
    {
        memcpy(p_cbv_data_begin_ + frame_index_ * kSizeConstantBufferPerFrame + kSizePerFrameConstantBuffer + index * kSizePerFrameConstantBuffer,
            &draw_batch_context_, sizeof(frame_index_));
        return true;
    }

    HRESULT Engine::D3d12GraphicsManager::CreateGraphicsResources()
    {
        HRESULT hr;
#if defined(_DEBUG)
        // Enable the D3D12 debug layer.
        {
            ID3D12Debug* pDebugController;
            if (SUCCEEDED(hr = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
            {
                pDebugController->EnableDebugLayer();
            }
            SafeRelease(&pDebugController);
        }
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
        hr = factory->CreateSwapChain(p_cmdqueue_.Get(), &scd,swapChain.GetAddressOf());
        ThrowIfFailed(hr = swapChain.As(&p_swapchain));
        frame_index_ = p_swapchain->GetCurrentBackBufferIndex();
        if (FAILED(hr = CreateDescriptorHeaps())) return hr; //2
        if (FAILED(hr = CreateRenderTarget())) return hr;
        if (FAILED(hr = CreateRootSignature())) return hr;
        if (FAILED(hr = InitializeShader("Shaders/simple_vs.cso", "Shaders/simple_ps.cso"))) return hr; //5
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
    HRESULT D3d12GraphicsManager::CreateTextureBuffer()
    {
        HRESULT hr;

        // Describe and create a Texture2D.
        D3D12_HEAP_PROPERTIES prop = {};
        prop.Type = D3D12_HEAP_TYPE_DEFAULT;
        prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        prop.CreationNodeMask = 1;
        prop.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Width = 1;
        textureDesc.Height = 1;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        if (FAILED(hr = p_device_->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr,
            IID_PPV_ARGS(p_texture_buf_.GetAddressOf()))))
        {
            return hr;
        }

        for (int32_t i = 0; i < kMaxTextureCount; i++)
        {
            // Describe and create a SRV for the texture.
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;
            srvHandle.ptr = p_cbv_heap_->GetCPUDescriptorHandleForHeapStart().ptr + (kTextureDescStartIndex + i) * cbv_srv_uav_desc_size_;
            p_device_->CreateShaderResourceView(p_texture_buf_.Get(), &srvDesc, srvHandle);
        }

        return hr;
    }
    HRESULT D3d12GraphicsManager::CreateConstantBuffer()
    {
        HRESULT hr;
        D3D12_HEAP_PROPERTIES prop = { D3D12_HEAP_TYPE_UPLOAD,
            D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            D3D12_MEMORY_POOL_UNKNOWN,
            1,
            1 };
        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = 0;
        resourceDesc.Width = kSizeConstantBufferPerFrame * kFrameCount;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        ComPtr<ID3D12Resource> pConstantUploadBuffer;
        if (FAILED(hr = p_device_->CreateCommittedResource(
            &prop,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(pConstantUploadBuffer.GetAddressOf()))))
        {
            return hr;
        }

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

        D3D12_RANGE readRange = { 0, 0 };
        hr = pConstantUploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&p_cbv_data_begin_));
        buffers_.push_back(pConstantUploadBuffer);
        return hr;
    }
    HRESULT D3d12GraphicsManager::InitializeBuffers()
    {
        HRESULT hr = S_OK;
        if (FAILED(hr = CreateDepthStencil())) {
            return hr;
        }
        if (FAILED(hr = CreateConstantBuffer())) {
            return hr;
        }
        if (FAILED(hr = CreateTextureBuffer())) {
            return hr;
        }
        if (FAILED(hr = CreateSamplerBuffer())) {
            return hr;
        }
        auto* scene = g_pSceneManager->GetSceneForRendering();
        if(scene != nullptr) {
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
                    auto vertexCount = pMesh->GetVertexCount();

                    Buffer buff;

                    for (decltype(vertexPropertiesCount) i = 0; i < vertexPropertiesCount; i++)
                    {
                        const SceneObjectVertexArray& v_property_array = pMesh->GetVertexPropertyArray(i);

                        CreateVertexBuffer(v_property_array);
                    }

                    auto indexGroupCount = pMesh->GetIndexGroupCount();

                    for (decltype(indexGroupCount) i = 0; i < indexGroupCount; i++)
                    {
                        const SceneObjectIndexArray& index_array = pMesh->GetIndexArray(i);

                        CreateIndexBuffer(index_array);
                    }

                    SetPerBatchShaderParameters(n);
                    n++;
                }
                pGeometryNode = scene->GetNextGeometryNode();
            }
            if (SUCCEEDED(hr = p_cmdlist_->Close()))
            {
                ID3D12CommandList* ppCommandLists[] = { p_cmdlist_.Get() };
                p_cmdqueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

                if (FAILED(hr = p_device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(p_fence_.GetAddressOf()))))
                {
                    return hr;
                }
                fence_value_ = 1;
                fence_event_ = CreateEvent(NULL, FALSE, FALSE, NULL);
                if (fence_event_ == NULL)
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    if (FAILED(hr))
                        return hr;
                }
                WaitForPreviousFrame();
            }
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
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            //{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            //{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };

        D3D12_RASTERIZER_DESC rsd = { D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK, TRUE, D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
                                      TRUE, FALSE, FALSE, 0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF };
        const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlend = { FALSE, FALSE,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
            D3D12_LOGIC_OP_NOOP,
            D3D12_COLOR_WRITE_ENABLE_ALL
        };

        D3D12_BLEND_DESC bld = { FALSE, FALSE,
                                                 {
                                                   defaultRenderTargetBlend,
                                                   defaultRenderTargetBlend,
                                                   defaultRenderTargetBlend,
                                                   defaultRenderTargetBlend,
                                                   defaultRenderTargetBlend,
                                                   defaultRenderTargetBlend,
                                                   defaultRenderTargetBlend,
                                                 }
        };

        const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = { D3D12_STENCIL_OP_KEEP,
            D3D12_STENCIL_OP_KEEP,
            D3D12_STENCIL_OP_KEEP,
            D3D12_COMPARISON_FUNC_ALWAYS };

        D3D12_DEPTH_STENCIL_DESC dsd = { TRUE,
            D3D12_DEPTH_WRITE_MASK_ALL,
            D3D12_COMPARISON_FUNC_LESS,
            FALSE,
            D3D12_DEFAULT_STENCIL_READ_MASK,
            D3D12_DEFAULT_STENCIL_WRITE_MASK,
            defaultStencilOp, defaultStencilOp };

        // describe and create the graphics pipeline state object (PSO)
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psod = {};
        psod.pRootSignature = p_rootsig_.Get();
        psod.VS = vertexShaderByteCode;
        psod.PS = pixelShaderByteCode;
        psod.BlendState = bld;
        psod.SampleMask = UINT_MAX;
        psod.RasterizerState = rsd;
        psod.DepthStencilState = dsd;
        psod.InputLayout = { ied, _countof(ied) };
        psod.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psod.NumRenderTargets = 1;
        psod.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psod.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        psod.SampleDesc.Count = 1;

        if (FAILED(hr = p_device_->CreateGraphicsPipelineState(&psod, IID_PPV_ARGS(p_plstate_.GetAddressOf()))))
        {
            return hr;
        }

        hr = p_device_->CreateCommandList(0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            p_cmdalloc_.Get(),
            p_plstate_.Get(),
            IID_PPV_ARGS(p_cmdlist_.GetAddressOf()));

        return hr;  
    }
    HRESULT D3d12GraphicsManager::RenderBuffers()
    {
        HRESULT hr;
        // execute the command list
        ID3D12CommandList* ppCommandLists[] = { p_cmdlist_.Get()};
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
        if (FAILED(hr = p_device_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(p_rtv_heap_.GetAddressOf())))) return hr;
        rtv_desc_size_ = p_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Describe and create a depth stencil view (DSV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
        dsvHeapDesc.NumDescriptors = 1;
        dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        if (FAILED(hr = p_device_->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(p_dsv_heap_.GetAddressOf())))) return hr;       
        // Describe and create a Shader Resource View (SRV) and 
        // Constant Buffer View (CBV) and 
        // Unordered Access View (UAV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC srv_cbv_uavHeapDesc = {};
        srv_cbv_uavHeapDesc.NumDescriptors = kFrameCount * (1 + kMaxSceneObjectCount) + kMaxTextureCount;
        srv_cbv_uavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srv_cbv_uavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (FAILED(hr = p_device_->CreateDescriptorHeap(&srv_cbv_uavHeapDesc, IID_PPV_ARGS(p_cbv_heap_.GetAddressOf())))) return hr;
        cbv_srv_uav_desc_size_ = p_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        // Describe and create a sampler descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
        samplerHeapDesc.NumDescriptors = 2048; // this is the max D3d12 HW support currently
        samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (FAILED(hr = p_device_->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(p_sampler_heap_.GetAddressOf())))) return hr;
        //Create CommandAllocator
        if (FAILED(hr = p_device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(p_cmdalloc_.GetAddressOf())))) return hr;
        return hr;
    }
    HRESULT Engine::D3d12GraphicsManager::CreateRenderTarget()
    {
        HRESULT hr;
        // Create descriptor heaps.
        rtv_desc_size_ = p_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(p_rtv_heap_->GetCPUDescriptorHandleForHeapStart());
        // Create a RTV for each frame.
        for (uint32_t i = 0; i < kFrameCount; i++)
        {
            hr = p_swapchain->GetBuffer(i, IID_PPV_ARGS(p_rt_[i].GetAddressOf()));
            ThrowIfFailed(hr);
            p_device_->CreateRenderTargetView(p_rt_[i].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1u,rtv_desc_size_);
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
}
