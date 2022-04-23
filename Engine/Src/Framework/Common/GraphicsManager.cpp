#include "pch.h"
#include "../Inc/Framework/Common/GraphicsManager.h"
#include "Framework/Common/Image.h"
#include "Framework/Common/GfxConfiguration.h"
#include "Framework/Interface/IApplication.h"
#include "Framework/Common/Log.h"


using namespace Engine;


int Engine::GraphicsManager::Initialize()
{
    int result = 0;
    InitConstants();
    p_cam_mgr_ = std::make_unique<CameraManager>();
    return result;
}

void Engine::GraphicsManager::Finalize()
{
#ifdef _DEBUG
    ClearDebugBuffers();
#endif // _DEBUG
    ClearBuffers();
    ClearShaders();
}

void Engine::GraphicsManager::Tick()
{
    if (g_pSceneManager->IsSceneChanged())
    {
        NE_LOG(ALL,kWarning,"[GraphicsManager] Detected Scene Change, reinitialize buffers ...")
        ClearBuffers();
        ClearShaders();
        auto& scene = g_pSceneManager->GetSceneForRendering();
        InitializeShaders();
        InitializeBuffers(scene);
        g_pSceneManager->NotifySceneIsRenderingQueued();
    }
    Clear();
    Draw();
}

void Engine::GraphicsManager::Clear()
{

}

void Engine::GraphicsManager::Draw()
{
    UpdateConstants();
    RenderBuffers();
#ifdef _DEBUG
    RenderDebugBuffers();
#endif //_DEBUG
}
void Engine::GraphicsManager::WorldRotateY(float radius)
{
    Matrix4x4f m;
    MatrixRotationZ(m,radius);
    draw_frame_context_.world_matrix_ = draw_frame_context_.world_matrix_ * m;
}
void Engine::GraphicsManager::MoveCameraForward(float distance)
{
   p_cam_mgr_->AddPositionOffset(0.f,0.f,distance);
}
void Engine::GraphicsManager::MoveCameraRight(float distance)
{
    p_cam_mgr_->AddPositionOffset(distance, 0.f, 0.f);
}
void Engine::GraphicsManager::CameraRotateYaw(float angle)
{
    p_cam_mgr_->GetCamera().RotateYaw(AngleToRadius(angle)); //->RotateYaw(AngleToRadius(angle));
}
void Engine::GraphicsManager::CameraRotatePitch(float angle)
{
    p_cam_mgr_->GetCamera().RotatePitch(AngleToRadius(angle));
}
#ifdef _DEBUG
void Engine::GraphicsManager::DrawLine(const Vector3f& from, const Vector3f& to, const Vector3f& color)
{
}

void Engine::GraphicsManager::DrawBox(const Vector3f& bbMin, const Vector3f& bbMax, const Vector3f& color)
{
}

void Engine::GraphicsManager::ClearDebugBuffers()
{
    NE_LOG(ALL, kNormal, "[GraphicsManager] GraphicsManager::ClearDebugBuffers()")
}
#endif


bool Engine::GraphicsManager::InitializeShaders()
{
    NE_LOG(ALL, kNormal, "[GraphicsManager] GraphicsManager::InitializeShaders()")
    return true;
}

void Engine::GraphicsManager::ClearShaders()
{
    NE_LOG(ALL, kNormal, "[GraphicsManager] GraphicsManager::ClearShaders()")
}

void Engine::GraphicsManager::InitializeBuffers(const Scene& scene)
{
    NE_LOG(ALL, kNormal, "[GraphicsManager] GraphicsManager::InitializeBuffers()")
}

void Engine::GraphicsManager::ClearBuffers()
{
    NE_LOG(ALL, kNormal, "[GraphicsManager] GraphicsManager::ClearBuffers()")
}


void Engine::GraphicsManager::InitConstants()
{
    BuildIdentityMatrix(draw_frame_context_.world_matrix_);
}

void Engine::GraphicsManager::CalculateCameraMatrix()
{
    const GfxConfiguration& conf = g_pApp->GetConfiguration();
    auto aspect = static_cast<float>(conf.viewport_width_) / static_cast<float>(conf.viewport_height_);
    //lenth is cm			
    draw_frame_context_.camera_position_ = Vector4f(p_cam_mgr_->GetCamera().GetPosition(),1.f);
    draw_frame_context_.view_matrix_ = Transpose(p_cam_mgr_->GetCamera().GetView());
    draw_frame_context_.projection_matrix_ = Transpose(p_cam_mgr_->GetCamera().GetProjection());
    draw_frame_context_.world_matrix_ = BuildIdentityMatrix();
    draw_frame_context_.ambient_color_ = Vector4f{ 0.1f,0.1f,0.1f,0.f };
    draw_frame_context_.light_color_ = Vector4f{ 1.f,1.f,1.f,1.f };
}

void Engine::GraphicsManager::CalculateLights()
{

}


void Engine::GraphicsManager::RenderBuffers()
{
    //NE_LOG(ALL, kNormal, "[GraphicsManager] GraphicsManager::RenderBuffers()")
}

void Engine::GraphicsManager::UpdateConstants()
{
    CalculateCameraMatrix();
    CalculateLights();
}

void Engine::GraphicsManager::RenderDebugBuffers()
{
    //NE_LOG(ALL, kNormal, "[GraphicsManager] GraphicsManager::RenderDebugBuffers()")
}
