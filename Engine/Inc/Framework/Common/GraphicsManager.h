#pragma once
#include "Framework/Interface/IRuntimeModule.h"
#include "SceneManager.h"
#include "Framework/Math/NutMath.hpp"
#include "Framework/Common/CameraManager.h"


namespace Engine 
{
	class GraphicsManager : public IRuntimeModule
	{
	public:
		virtual ~GraphicsManager() {}
		virtual int Initialize();
		virtual void Finalize();
		virtual void Tick();
		virtual void Clear();
		virtual void Draw();
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
#endif
	protected:
		virtual bool InitializeShaders();
		virtual void ClearShaders();
		virtual void InitializeBuffers(const Scene& scene);
		virtual void ClearBuffers();

		virtual void InitConstants();
		virtual void CalculateCameraMatrix();
		virtual void CalculateLights();
		virtual void RenderBuffers();
		virtual void UpdateConstants();
#ifdef _DEBUG
		virtual void RenderDebugBuffers();
#endif
	protected:
		struct DrawFrameContext {
			Matrix4x4f  world_matrix_;
			Matrix4x4f  view_matrix_;
			Matrix4x4f  projection_matrix_;
			Vector4f    ambient_color_;
			Vector4f    light_color_;
			Vector4f    light_position_;
			Vector4f	camera_position_;
		};
		DrawFrameContext    draw_frame_context_;
		//temp
		std::unique_ptr<CameraManager> p_cam_mgr_;
	};
	extern GraphicsManager* g_pGraphicsManager;
}