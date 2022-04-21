#include "pch.h"
#include "../Inc/Framework/Common/GraphicsManager.h"
#include "Framework/Common/Image.h"
#include "Framework/Common/GfxConfiguration.h"
#include "Framework/Interface/IApplication.h"


using namespace Engine;


int Engine::GraphicsManager::Initialize()
{
    int result = 0;
    InitConstants();
    return result;
}

void Engine::GraphicsManager::Finalize()
{
}

void Engine::GraphicsManager::Tick()
{
    if (g_pSceneManager->IsSceneChanged())
    {
        //TODO:Lgger
        //cout << "[GraphicsManager] Detected Scene Change, reinitialize buffers ..." << endl;
        Finalize();
        Initialize();
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
}

#endif

bool Engine::GraphicsManager::SetPerFrameShaderParameters()
{
    return false;
}
bool Engine::GraphicsManager::SetPerBatchShaderParameters(const char* paramName, const Matrix4x4f& param)
{
    return false;
}
bool Engine::GraphicsManager::SetPerBatchShaderParameters(const char* paramName, const Vector3f& param)
{
    return false;
}
bool Engine::GraphicsManager::SetPerBatchShaderParameters(const char* paramName, const float param)
{
    return false;
}
bool Engine::GraphicsManager::SetPerBatchShaderParameters(const char* paramName, const int param)
{
    return false;
}

bool Engine::GraphicsManager::InitializeShaders(const char* vsFilename, const char* fsFilename)
{
    return false;
}

void Engine::GraphicsManager::InitializeBuffers()
{
}


void Engine::GraphicsManager::InitConstants()
{
    BuildIdentityMatrix(draw_frame_context_.world_matrix_);
    CalculateCameraMatrix();
}

void Engine::GraphicsManager::CalculateCameraMatrix()
{
    auto* scene = g_pSceneManager->GetSceneForRendering();
    if(scene == nullptr) return;
    auto pCameraNode = scene->GetFirstCameraNode();
    if (pCameraNode) {
        draw_frame_context_.view_matrix_ = *pCameraNode->GetCalculatedTransform();
        //InverseMatrix4X4f(m_DrawFrameContext.m_viewMatrix);
    }
    else {
        // use default build-in camera
        Vector3f position = { 0, -5, 0 }, lookAt = { 0, 0, 0 }, up = { 0, 0, 1 };
        BuildViewMatrixLookAtLH(draw_frame_context_.view_matrix_, position, lookAt, up);
    }
    float fieldOfView = kPi / 2.0f;
    float nearClipDistance = 1.0f;
    float farClipDistance = 100.0f;
    //if (pCameraNode) {
    //    auto pCamera = scene.GetCamera(pCameraNode->GetSceneObjectRef());
    //    // Set the field of view and screen aspect ratio.
    //    fieldOfView = dynamic_pointer_cast<SceneObjectPerspectiveCamera>(pCamera)->GetFov();
    //    nearClipDistance = pCamera->GetNearClipDistance();
    //    farClipDistance = pCamera->GetFarClipDistance();
    //}

    const GfxConfiguration& conf = g_pApp->GetConfiguration();

    float screenAspect = static_cast<float>(conf.viewport_width_) / static_cast<float>(conf.viewport_height_);
    // Build the perspective projection matrix.
    BuildPerspectiveFovLHMatrix(draw_frame_context_.projection_matrix_, fieldOfView, screenAspect, nearClipDistance, farClipDistance);
}

void Engine::GraphicsManager::CalculateLights()
{
    auto* scene = g_pSceneManager->GetSceneForRendering();
    auto pLightNode = scene->GetFirstLightNode();
    if (pLightNode) {
        //draw_frame_context_.light_position_ = { 0.0f, 0.0f, 0.0f };
        //TransformCoord(draw_frame_context_.light_position_, *pLightNode->GetCalculatedTransform());

        //auto pLight = scene->GetLight(pLightNode->GetSceneObjectRef());
        //if (pLight) {
        //   // draw_frame_context_.light_color_ = pLight->GetColor().value_.xyz;
        //}
    }
    else {
        // use default build-in light 
        //draw_frame_context_.light_position_ = { -1.0f, -5.0f, 0.0f };
        //draw_frame_context_.light_color_ = { 1.0f, 1.0f, 1.0f};
    }
}


void Engine::GraphicsManager::RenderBuffers()
{
}
