#include "pch.h"
#include "../Inc/Framework/Common/GraphicsManager.h"
#include "Framework/Common/Image.h"
#include "Framework/Common/GfxConfiguration.h"
#include "Framework/Interface/IApplication.h"
#include "Framework/Common/Log.h"
#include "Framework/DrawPass/ForwardRenderPass.h"


using namespace Engine;


int Engine::GraphicsManager::Initialize()
{
    int result = 0;
    frames_.resize(kFrameCount);
    InitConstants();
    draw_passes_.emplace_back(std::make_shared<ForwardRenderPass>());
    p_cam_mgr_ = std::make_unique<CameraManager>();
    return result;
}

void Engine::GraphicsManager::Finalize()
{
#ifdef _DEBUG
    ClearDebugBuffers();
#endif // _DEBUG
    EndScene();
}

void Engine::GraphicsManager::Tick()
{
    if (g_pSceneManager->IsSceneChanged())
    {
        //EndScene();
        NE_LOG(ALL,kWarning,"[GraphicsManager] Detected Scene Change, reinitialize buffers ...")
        auto& scene = g_pSceneManager->GetSceneForRendering();
        BeginScene(scene);
        g_pSceneManager->NotifySceneIsRenderingQueued();
    }
    UpdateConstants();

    BeginFrame();
    Draw();
    EndFrame();

    Present();
}


void Engine::GraphicsManager::Draw()
{
    auto& frame = frames_[0];
    for(auto p_pass : draw_passes_)
    {
        p_pass->Draw(frame);
    }    
//#ifdef _DEBUG
//    RenderDebugBuffers();
//#endif //_DEBUG
}
void Engine::GraphicsManager::Present()
{
}
void Engine::GraphicsManager::UseShaderProgram(const INT32 shaderProgram)
{
}
void Engine::GraphicsManager::SetPerFrameConstants(DrawFrameContext& context)
{
}
void Engine::GraphicsManager::SetPerBatchConstants(std::vector<std::shared_ptr<DrawBatchContext>>& batches)
{
}
void Engine::GraphicsManager::DrawBatch(const std::vector<std::shared_ptr<DrawBatchContext>>& batches)
{

}
void Engine::GraphicsManager::WorldRotateY(float radius)
{
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


void Engine::GraphicsManager::BeginScene(const Scene& scene)
{
    NE_LOG(ALL, kNormal, "[GraphicsManager] GraphicsManager::BeginScene()")
}

void Engine::GraphicsManager::EndScene()
{
    NE_LOG(ALL, kNormal, "[GraphicsManager] GraphicsManager::EndScene()")
}

void Engine::GraphicsManager::BeginFrame()
{
}

void Engine::GraphicsManager::EndFrame()
{
}


void Engine::GraphicsManager::InitConstants()
{
    BuildIdentityMatrix(frames_[frame_index_].frame_context.world_matrix_);
}

void Engine::GraphicsManager::CalculateCameraMatrix()
{
    auto& draw_frame_context_ = frames_[frame_index_].frame_context;
    const GfxConfiguration& conf = g_pApp->GetConfiguration();
    auto aspect = static_cast<float>(conf.viewport_width_) / static_cast<float>(conf.viewport_height_);
    //lenth is cm			
    draw_frame_context_.camera_position_ = p_cam_mgr_->GetCamera().GetPosition();
    draw_frame_context_.view_matrix_ = Transpose(p_cam_mgr_->GetCamera().GetView());
    draw_frame_context_.projection_matrix_ = Transpose(p_cam_mgr_->GetCamera().GetProjection());
    draw_frame_context_.world_matrix_ = BuildIdentityMatrix();
}

void Engine::GraphicsManager::CalculateLights()
{
    auto& draw_frame_context_ = frames_[frame_index_].frame_context;
    auto& scene = g_pSceneManager->GetSceneForRendering();
    if(scene.GetFirstLightNode())
    {
        int i = 0;
        for (auto& node : scene.LightNodes)
        {
            if (auto light = scene.GetLight(node.second->GetSceneObjectRef()); light != nullptr)
            {
                Light single_light;
                single_light.light_color = light->GetColor().value_.xyz;
                single_light.light_direction = node.second->GetForwardDir();
                single_light.light_position = node.second->GetWorldPosition();
                single_light.light_instensity = light->GetIntensity() / 200000.f;
                single_light.falloff_begin = 800.f;
                single_light.falloff_end = 5000.f;
                single_light.is_able = 1.f;
                switch (light->GetType())
                {
                case Engine::ELightType::kDirectional:
                {
                    single_light.type = 0;
                }
                    break;
                case Engine::ELightType::kPoint:
                {
                    single_light.type = 1;
                }
                    break;
                case Engine::ELightType::kSpot:
                {
                    single_light.type = 2;
                    auto spot_light = std::dynamic_pointer_cast<SceneObjectSpotLight>(light);
                    single_light.inner_angle = spot_light->inner_angle_;
                    single_light.outer_angle = spot_light->outer_angle_;
                }
                    break;
                default:
                    break;
                }
                draw_frame_context_.lights_[i++] = single_light;
            }
        }
    }
    else
    {

    }
    draw_frame_context_.ambient_color_ = kDefaultAmbientLight;
}


void Engine::GraphicsManager::UpdateConstants()
{
    auto& frame = frames_[frame_index_];
    CalculateCameraMatrix();
    CalculateLights();
    SetPerFrameConstants(frame.frame_context);
    SetPerBatchConstants(frame.batch_contexts);
}

void Engine::GraphicsManager::RenderDebugBuffers()
{
    //NE_LOG(ALL, kNormal, "[GraphicsManager] GraphicsManager::RenderDebugBuffers()")
}
