#pragma once
#include "pch.h"
#include "../Inc/RHI/D3D12GrahpicsManager.h"
#include "../Inc/Framework/Interface/IApplication.h"


namespace Engine
{
    extern IApplication* g_pApp;
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

    int Engine::D3d12GraphicsManager::Initialize()
    {
        int ret = 0;
        ret = static_cast<int>(CreateGraphicsResources());
        return ret;
    }

    void Engine::D3d12GraphicsManager::Finalize()
    {

    }

    void Engine::D3d12GraphicsManager::Tick()
    {
    }

    HRESULT Engine::D3d12GraphicsManager::CreateGraphicsResources()
    {
        HRESULT hr;
#if defined(_DEBUG)
        // Enable the D3D12 debug layer.
        {
            ID3D12Debug* pDebugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController))))
            {
                pDebugController->EnableDebugLayer();
            }
            SafeRelease(&pDebugController);
        }
#endif
        ComPtr<IDXGIFactory4> factory;
        ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
        ComPtr<IDXGIAdapter1> hardwareAdapter;
        GetHardwareAdapter(factory.Get(), hardwareAdapter.GetAddressOf());
        if (FAILED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(p_device_.GetAddressOf()))))
        {
            IDXGIAdapter* pWarpAdapter;
            if (FAILED(hr = factory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter))))
                return hr;
        }
        ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(p_device_.GetAddressOf())));

        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ThrowIfFailed(p_device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(p_cmdqueue_.GetAddressOf())));

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
        scd.BufferCount = ks_FrameCount_;
        scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // DXGI_SWAP_EFFECT_FLIP_DISCARD only supported after Win10
        scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        scd.OutputWindow = hwnd;

        ComPtr<IDXGISwapChain> swapChain;
        // Swap chain needs the queue so that it can force a flush on it.
        hr = factory->CreateSwapChain(p_cmdqueue_.Get(), &scd,swapChain.GetAddressOf());
        ThrowIfFailed(swapChain.As(&p_swapchain));
        frame_index_ = p_swapchain->GetCurrentBackBufferIndex();
        hr = CreateRenderTarget();
        return hr;
    }

    HRESULT Engine::D3d12GraphicsManager::CreateRenderTarget()
    {
        HRESULT hr;
        // Create descriptor heaps.
        {
            // Describe and create a render target view (RTV) descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = ks_FrameCount_;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            hr = p_device_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(p_rtv_heap_.GetAddressOf()));
            ThrowIfFailed(hr);

            rtv_desc_size_ = p_device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(p_rtv_heap_->GetCPUDescriptorHandleForHeapStart());
            // Create a RTV for each frame.
            for (uint32_t i = 0; i < ks_FrameCount_; i++)
            {
                hr = p_swapchain->GetBuffer(i, IID_PPV_ARGS(p_rt_[i].GetAddressOf()));
                ThrowIfFailed(hr);
                p_device_->CreateRenderTargetView(p_rt_[i].Get(), nullptr, rtvHandle);
                rtvHandle.Offset(1u,rtv_desc_size_);
            }
        }
        return hr;
    }
}
