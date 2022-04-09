#pragma once
#include "Framework/Interface/IRuntimeModule.h"
#include "SceneManager.h"
#include "Framework/Math/NutMath.hpp"


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
#ifdef DEBUG
		virtual void DrawLine(const Vector3f& from, const Vector3f& to, const Vector3f& color);
		virtual void DrawBox(const Vector3f& bbMin, const Vector3f& bbMax, const Vector3f& color);
		virtual void ClearDebugBuffers();
#endif
	protected:
		bool SetPerFrameShaderParameters();
		bool SetPerBatchShaderParameters(const char* paramName, const Matrix4x4f& param);
		bool SetPerBatchShaderParameters(const char* paramName, const Vector3f& param);
		bool SetPerBatchShaderParameters(const char* paramName, const float param);
		bool SetPerBatchShaderParameters(const char* paramName, const int param);

		bool InitializeShaders(const char* vsFilename, const char* fsFilename);
		void InitializeBuffers();
		void InitConstants();
		void CalculateCameraMatrix();
		void CalculateLights();
		void RenderBuffers();
	protected:
		struct DrawFrameContext {
			Matrix4x4f  world_matrix_;
			Matrix4x4f  view_matrix_;
			Matrix4x4f  projection_matrix_;
			Vector3f    light_position_;
			Vector4f    light_color_;
		};
		DrawFrameContext    draw_frame_context_;
	};
	extern GraphicsManager* g_pGraphicsManager;
}