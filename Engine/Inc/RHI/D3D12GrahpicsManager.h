#pragma once

#include "../../pch.h"
#include <array>
#include "Framework/Common/GraphicsManager.h"
#include "Framework/Common/SceneObject.h"
#include "Framework/Common/Buffer.h"

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
        struct DrawDebugBatchContext
        {
            INT32 count;
            INT32 vertex_buf_start_;
            INT32 vertex_buf_len_;
            INT32 vertex_buf_id_;
        };
    public:
        int Initialize() final;
        void Finalize() final;
        void Present() final;
        void Draw() final;

        void UseShaderProgram(const INT32 shaderProgram) final;

        void DrawBatch(const std::vector<std::shared_ptr<DrawBatchContext>>& batches) final;
        void DrawBatch(std::shared_ptr<DrawBatchContext> batch) final;

#ifdef _DEBUG
        void DrawLine(const Vector3f& from, const Vector3f& to, const Vector3f& color) final;
        void DrawBox(const Vector3f& bbMin, const Vector3f& bbMax, const Vector3f& color) final;
        void DrawOverlay();
    private:
        void ClearDebugBuffers() final;
        void ClearVertexData();
        void InitializeBufferDebug();
        void InitializeShaderDebug();
#endif

    enum class EBufferType
    {
        kNormal,kHUD
    };
    private:
        void BeginScene(const Scene& scene) final;
        void EndScene() final;

        void BeginFrame() final;
        void EndFrame() final;

        void GenerateShadowMapArray(UINT32 count = kMaxLightNum);
        void BeginShadowMap(Light& light, int light_id, int point_light_id, int cube_map_id) final;
        void EndShadowMap(int light_index, int point_light_id, bool is_point_light, bool final) final;
        void SetShadowMap();
        void DestroyShadowMap(intptr_t& shadowmap);
        void BeginRenderPass() final;

        HRESULT ResetCommandList();
        HRESULT CreateCommandList();
        HRESULT InitializePSO();

        HRESULT CreateDescriptorHeaps();
        HRESULT CreateRenderTarget();
        HRESULT CreateDepthStencil();
        HRESULT CreateGraphicsResources();
        HRESULT CreateSamplerBuffer();
        HRESULT CreateTextureBuffer(SceneObjectTexture& texture);
        HRESULT CreateConstantBuffer();
        HRESULT CreateIndexBuffer(const SceneObjectIndexArray& index_array);
        HRESULT CreateVertexBuffer(const SceneObjectVertexArray& vertex_array, EBufferType type = EBufferType::kNormal);
        HRESULT CreateRootSignature();
        HRESULT WaitForPreviousFrame();

        virtual void SetPerFrameConstants(DrawFrameContext& context) final;
        virtual void SetPerBatchConstants(std::vector<std::shared_ptr<DrawBatchContext>>& batches) final;

    private:
        struct D3dDrawBatchContext : public DrawBatchContext
        {
            INT32 count;
            INT32 vertex_buf_start;
            INT32 vertex_buf_len;
        };
        static constexpr uint32_t		    kTextureDescStartIndex = kFrameCount * (1 + kMaxSceneObjectCount);
        static constexpr FLOAT              kBackColor[] = { 0.5f, 0.0f, 0.0f, 1.0f };

        ComPtr<ID3D12Resource> p_shadow_map_ = nullptr;
        ComPtr<ID3D12PipelineState> p_plstate_sm_ = nullptr;
        
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
        uint32_t                        dsv_desc_size_;
        uint32_t                        cbv_srv_uav_desc_size_;
        uint32_t                        vertex_buf_per_frame_num_;

        uint32_t                        cube_shadow_map_srv_start_ = 0u;
        uint32_t                        cur_point_light_index = 0u;

        uint32_t                        shadow_map_start_;      //start pos of the shadow_map based on the srv handle
        uint32_t                        shadow_map_buf_start_;      //start pos of the shadow_map based on the srv handle

        ComPtr<ID3D12Resource> p_vertex_buf_ = nullptr;          // the pointer to the vertex buffer
        std::vector<ComPtr<ID3D12Resource>>    buffers_;                          // the pointer to the vertex buffer
        std::vector<D3D12_VERTEX_BUFFER_VIEW>       vertex_buf_view_;                 // a view of the vertex buffer
        std::vector<D3D12_INDEX_BUFFER_VIEW>        index_buf_view_;                  // a view of the index buffer

        std::vector<ComPtr<ID3D12Resource>> textures_;
        std::map<std::string,INT32> texture_index_;
#ifdef _DEBUG
        UINT8* p_vex_data_begin = nullptr;
        UINT8* p_color_data_begin = nullptr;
        std::vector<DrawDebugBatchContext> draw_batch_contexts_debug_;
        ComPtr<ID3D12PipelineState> p_plstate_debug_ = nullptr;
        std::vector<ComPtr<ID3D12Resource>>    buffers_debug_;
        std::vector<D3D12_VERTEX_BUFFER_VIEW>       vertex_buf_view_debug_;
        Vector3f* vertex_data_debug_ = nullptr;
        Vector3f* color_data_debug_ = nullptr;
        int cur_debug_vertex_pos = 0;

        std::vector<ComPtr<ID3D12Resource>>    buffers_hud_;
        std::vector<D3D12_VERTEX_BUFFER_VIEW>       vertex_buf_view_hud_;
        ComPtr<ID3D12PipelineState> p_plstate_hud_= nullptr;
#endif
        uint8_t* p_cbv_data_begin_ = nullptr;
        // CB size is required to be 256-byte aligned. make the size of DrawFrameContext and it's member 16bit align
        static constexpr size_t				kSizePerBatchConstantBuffer = ALIGN(sizeof(PerBatchConstants),256);
        static constexpr size_t				kSizePerFrameConstantBuffer =ALIGN(sizeof(DrawFrameContext),256); // CB size is required to be 256-byte aligned.
        static constexpr size_t				kSizeConstantBufferPerFrame = kSizePerFrameConstantBuffer + kSizePerBatchConstantBuffer * kMaxSceneObjectCount;
        // Synchronization objects
        HANDLE                          fence_event_;
        ComPtr<ID3D12Fence> p_fence_    = nullptr;
        uint32_t                        fence_value_;
    };
}


