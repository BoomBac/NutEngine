#pragma once
#include "Framework/Interface/IRuntimeModule.h"
#include "SceneManager.h"
#include "Framework/Math/NutMath.hpp"
#include "Framework/Common/CameraManager.h"
#include "../Interface/IDrawPass.h"


namespace Engine 
{
	class GraphicsManager : public IRuntimeModule
	{
	public:
		virtual ~GraphicsManager() {}
		virtual int Initialize();
		virtual void Finalize();
		virtual void Tick();
		virtual void Draw();
		virtual void Present();

		virtual void UseShaderProgram(const INT32 shaderProgram);

		virtual void DrawBatch(const std::vector<std::shared_ptr<DrawBatchContext>>& batches);

		virtual void DrawBatch(std::shared_ptr<DrawBatchContext> batch);

		virtual void GenerateShadowMapArray(UINT32 count);
		virtual void BeginShadowMap(Light& light, int light_id, int point_light_id = 0, int cube_map_id = 0);

		virtual void EndShadowMap(int light_index, bool is_point_light = false, int point_light_id = 0);
		virtual void EndShadowMap();

		virtual void SetShadowMap();

		//temp just adjust view_matrix form light to camera
		virtual void BeginRenderPass();

		bool REGenerateShadowMap() const {return b_regenerate_shadow_map_;}
		//temp
		void WorldRotateY(float radius);
		void MoveCameraForward(float distance);
		void MoveCameraRight(float distance);
		void CameraRotateYaw(float angle);
		void CameraRotatePitch(float angle);
#ifdef _DEBUG
		virtual void DrawLine(const Vector3f& from, const Vector3f& to, const Vector3f& color);
		virtual void DrawBox(const Vector3f& bbMin, const Vector3f& bbMax, const Vector3f& color);
		virtual void ClearDebugBuffers();

		virtual void DrawOverlay();
#endif
	protected:
		virtual void BeginScene(const Scene& scene);
		virtual void EndScene();

		virtual void BeginFrame();
		virtual void EndFrame();

		virtual void InitConstants();
		virtual void CalculateCameraMatrix();
		virtual void CalculateLights();
		virtual void UpdateConstants();

		virtual void SetPerFrameConstants(DrawFrameContext& context);
		virtual void SetPerBatchConstants(std::vector<std::shared_ptr<DrawBatchContext>>& batches);

#ifdef _DEBUG
		virtual void RenderDebugBuffers();
#endif
	protected:					
		static constexpr uint32_t           kFrameCount = 2;
		static constexpr uint32_t           kMaxSceneObjectCount = 65535;
		static constexpr uint32_t           kMaxTextureCount = 2048;
		static constexpr uint32_t           kMaxShadowMapCount = (kMaxLightNum - kMaxPointLightNum) + kMaxPointLightNum * 6;
		static constexpr uint32_t           kDefalutCubeShadowMapSize = 1024;

		UINT32 frame_index_ = 0;
		std::vector<Frame> frames_;
		std::vector<std::shared_ptr<IDrawPass>> draw_passes_;
		//temp
		std::unique_ptr<CameraManager> p_cam_mgr_;

		inline static const Vector3f			kOrigin = { 0.f,0.f,0.f };
		inline static const Vector3f			kForward = { 0.f,0.f,100000.f };
		inline static const Vector3f			kRight = { 100000.f,0.f,0.f };
		inline static const Vector3f			kUp = { 0.f,100000.f,0.f };

		inline static const Vector4f			kDefaultLightColor = { 1.f,1.f,1.f,1.f};
		inline static const Vector4f			kDefaultAmbientLight = { 0.0f,0.0f,0.0f,1.f};

		bool b_regenerate_shadow_map_ = true;
	};
	extern GraphicsManager* g_pGraphicsManager;
}