#pragma once

#include "../../pch.h"
#include <DirectXMath.h>
#include "Framework/Common/GraphicsManager.h"
#include "Framework/Common/SceneObject.h"

using Microsoft::WRL::ComPtr;

namespace Engine
{
    class D3d12GraphicsManager : public GraphicsManager
    {
        struct PerBatchConstants
        {
            Matrix4x4f object_matrix;
            Matrix4x4f normal_matrix;
            Vector4f base_color;
            Vector4f specular_color;
            float specular_power;
            float use_texture;
        };
        struct DrawBatchContext
        {
            INT32 count;
            INT32 vertex_buf_start_;
            INT32 vertex_buf_len_;
            std::shared_ptr<SceneGeometryNode> node;
            std::shared_ptr<SceneObjectMaterial> material;
        };
    public:
        int Initialize() override;
        void Finalize() override;
        void Tick() override;
        void Clear() override;
        void Draw() override;
        void Update();
#ifdef _DEBUG
        void DrawLine(const Vector3f& from, const Vector3f& to, const Vector3f& color) override;
        void DrawBox(const Vector3f& bbMin, const Vector3f& bbMax, const Vector3f& color) override;
        void ClearDebugBuffers() override;
#endif
    protected:
        bool SetPerFrameShaderParameters();
        bool SetPerBatchShaderParameters(int32_t index);
        HRESULT InitializeBuffers();
        HRESULT InitializeShader(const char* vs_filename, const char* fs_filename);
        HRESULT RenderBuffers();
    private:
        HRESULT CreateDescriptorHeaps();
        HRESULT CreateRenderTarget();
        HRESULT CreateDepthStencil();
        HRESULT CreateGraphicsResources();
        HRESULT CreateSamplerBuffer();
        HRESULT CreateTextureBuffer(SceneObjectTexture& texture);
        HRESULT CreateConstantBuffer();
        HRESULT CreateIndexBuffer(const SceneObjectIndexArray& index_array);
        HRESULT CreateVertexBuffer(const SceneObjectVertexArray& vertex_array);
        HRESULT CreateRootSignature();
        HRESULT WaitForPreviousFrame();
        HRESULT PopulateCommandList();

    private:
        static constexpr uint32_t           kFrameCount = 2;
        static constexpr uint32_t           kMaxSceneObjectCount = 65535;
        static constexpr uint32_t           kMaxTextureCount = 2048;
        static constexpr uint32_t		    kTextureDescStartIndex = kFrameCount * (1 + kMaxSceneObjectCount);
        static constexpr FLOAT              kBackColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        
        ComPtr<ID3D12Device> p_device_ = nullptr;             // the pointer to our Direct3D device interface
        D3D12_VIEWPORT                  vp_;                         // viewport structure
        D3D12_RECT                      rect_;                      // scissor rect structure
        ComPtr<ID3D12Resource> p_ds_buffer_ = nullptr;
        ComPtr<IDXGISwapChain3> p_swapchain = nullptr;             // the pointer to the swap chain interface
        ComPtr<ID3D12Resource> render_target_arr_[kFrameCount];      // the pointer to rendering buffer. [descriptor]
        ComPtr<ID3D12CommandAllocator> p_cmdalloc_ = nullptr;      // the pointer to command buffer allocator
        ComPtr<ID3D12CommandQueue> p_cmdqueue_ = nullptr;          // the pointer to command queue
        ComPtr<ID3D12RootSignature> p_rootsig_ = nullptr;         // a graphics root signature defines what resources are bound to the pipeline
        ComPtr<ID3D12DescriptorHeap> p_rtv_heap_ = nullptr;               // an array of descriptors of GPU objects
        ComPtr<ID3D12DescriptorHeap> p_dsv_heap_ = nullptr;               // an array of descriptors of GPU objects
        ComPtr<ID3D12DescriptorHeap> p_cbv_heap_ = nullptr;               // an array of descriptors of GPU objects
        ComPtr<ID3D12DescriptorHeap> p_sampler_heap_ = nullptr;               // an array of descriptors of GPU objects
        ComPtr<ID3D12PipelineState> p_plstate_ = nullptr;         // an object maintains the state of all currently set shaders
                                                                            // and certain fixed function state objects
                                                                            // such as the input assembler, tesselator, rasterizer and output manager
        ComPtr<ID3D12GraphicsCommandList> p_cmdlist_ = nullptr;           // a list to store GPU commands, which will be submitted to GPU to execute when done

        uint32_t                        rtv_desc_size_;
        uint32_t                        cbv_srv_uav_desc_size_;
        uint32_t                        vertex_buf_per_frame_num_;

        ComPtr<ID3D12Resource> p_vertex_buf_ = nullptr;          // the pointer to the vertex buffer
        std::vector<ComPtr<ID3D12Resource>>    buffers_;                          // the pointer to the vertex buffer
        std::vector<D3D12_VERTEX_BUFFER_VIEW>       vertex_buf_view_;                 // a view of the vertex buffer
        std::vector<D3D12_INDEX_BUFFER_VIEW>        index_buf_view_;                  // a view of the index buffer
        std::vector<PerBatchConstants> draw_batch_constants_;
        std::vector<DrawBatchContext> draw_batch_contexts_;
        std::vector<ComPtr<ID3D12Resource>> textures_;
        std::map<std::string,INT32> texture_index_;
        
        uint8_t* p_cbv_data_begin_ = nullptr;
        // CB size is required to be 256-byte aligned.
        static constexpr size_t				kSizePerBatchConstantBuffer = (sizeof(PerBatchConstants) + 255) & 256;
        static constexpr size_t				kSizePerFrameConstantBuffer = (sizeof(DrawFrameContext) + 255) & 256; // CB size is required to be 256-byte aligned.
        static constexpr size_t				kSizeConstantBufferPerFrame = kSizePerFrameConstantBuffer + kSizePerBatchConstantBuffer * kMaxSceneObjectCount;
        // Synchronization objects
        uint32_t                        cur_back_buf_index_;
        HANDLE                          fence_event_;
        ComPtr<ID3D12Fence> p_fence_ = nullptr;
        uint32_t                        fence_value_;
    };
}


