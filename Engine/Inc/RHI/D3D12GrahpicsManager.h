#pragma once

#include "../../pch.h"
#include "../Framework/Common/GraphicsManager.h"

using Microsoft::WRL::ComPtr;

namespace Engine
{
    class D3d12GraphicsManager : public GraphicsManager
    {
    public:
        virtual int Initialize();
        virtual void Finalize();
        virtual void Tick();
    private:
        HRESULT CreateRenderTarget();
        HRESULT CreateGraphicsResources();
    private:
        static constexpr uint32_t           ks_FrameCount_ = 2;
        
        ComPtr<ID3D12Device> p_device_ = nullptr;             // the pointer to our Direct3D device interface
        D3D12_VIEWPORT                  vp_;                         // viewport structure
        D3D12_RECT                      rect_;                      // scissor rect structure
        ComPtr<IDXGISwapChain3> p_swapchain = nullptr;             // the pointer to the swap chain interface
        ComPtr<ID3D12Resource> p_rt_[ks_FrameCount_];      // the pointer to rendering buffer. [descriptor]
        ComPtr<ID3D12CommandAllocator> p_cmdalloc_ = nullptr;      // the pointer to command buffer allocator
        ComPtr<ID3D12CommandQueue> p_cmdqueue_ = nullptr;          // the pointer to command queue
        ComPtr<ID3D12RootSignature> p_rootsig_ = nullptr;         // a graphics root signature defines what resources are bound to the pipeline
        ComPtr<ID3D12DescriptorHeap> p_rtv_heap_ = nullptr;               // an array of descriptors of GPU objects
        ComPtr<ID3D12PipelineState> p_plstate_ = nullptr;         // an object maintains the state of all currently set shaders
                                                                            // and certain fixed function state objects
                                                                            // such as the input assembler, tesselator, rasterizer and output manager
        ComPtr<ID3D12GraphicsCommandList> p_gfxcmdlist_ = nullptr;           // a list to store GPU commands, which will be submitted to GPU to execute when done

        uint32_t                        rtv_desc_size_;

        ID3D12Resource* p_vertex_buf_ = nullptr;          // the pointer to the vertex buffer
        D3D12_VERTEX_BUFFER_VIEW        p_vertex_bufv__;                 // a view of the vertex buffer

        // Synchronization objects
        uint32_t                        frame_index_;
        HANDLE                          fence_event_;
        ComPtr<ID3D12Fence> p_fence_ = nullptr;
        uint32_t                        fence_value_;
    };
}
