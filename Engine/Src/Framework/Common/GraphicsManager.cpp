#include "pch.h"
#include "../Inc/Framework/Common/GraphicsManager.h"
#include "Framework/Common/Image.h"
#include "Framework/Common/GfxConfiguration.h"
#include "Framework/Interface/IApplication.h"
#include "Framework/Common/Log.h"
#include "Framework/DrawPass/ForwardRenderPass.h"
#include "Framework/DrawPass/ShadowMapPass.h"
#include "Framework/DrawPass/HUDPass.h"

#include <directxmath.h>


using namespace Engine;


int Engine::GraphicsManager::Initialize()
{
    int result = 0;
    frames_.resize(kFrameCount);
    InitConstants();
    draw_passes_.emplace_back(std::make_shared<ShadowMapPass>());
    draw_passes_.emplace_back(std::make_shared<ForwardRenderPass>());
    draw_passes_.emplace_back(std::make_shared<HUDPass>());
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

void Engine::GraphicsManager::DrawBatch(std::shared_ptr<DrawBatchContext> batch)
{
}

void Engine::GraphicsManager::GenerateShadowMapArray(UINT32 count)
{

}
void Engine::GraphicsManager::BeginShadowMap(Light& light, int light_id, int point_light_id, int cube_map_id)
{
    b_regenerate_shadow_map_ = false;
}
void Engine::GraphicsManager::EndShadowMap(int light_index, bool is_point_light, int point_light_id)
{
}
void Engine::GraphicsManager::EndShadowMap()
{
}
void Engine::GraphicsManager::SetShadowMap()
{
}

void Engine::GraphicsManager::BeginRenderPass()
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
void Engine::GraphicsManager::DrawOverlay()
{
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
    draw_frame_context_.vp_matrix_ = Transpose(p_cam_mgr_->GetCamera().GetView());
    draw_frame_context_.projection_matrix_ = Transpose(p_cam_mgr_->GetCamera().GetProjection());
    draw_frame_context_.world_matrix_ = BuildIdentityMatrix();
}

void Engine::GraphicsManager::CalculateLights()
{
    static const Vector3f kForward[6]{ 
        {1.f,0.f,0.f}, //+x
        {-1.f,0.f,0.f}, //-x
        {0.f,1.f,0.f}, //+y
        {0.f,-1.f,0.f}, //-y
        {0.f,0.f,1.f}, //+z
        {0.f,0.f,-1.f}, //-z
    };
    static const Vector3f kUpDirs[6]{
        {0.f,1.f,0.f}, //+x
        {0.f,1.f,0.f}, //-x
        {0.f,0.f,-1.f}, //+y
        {0.f,0.f,1.f}, //-y
        {0.f,1.f,0.f}, //+z
        {0.f,1.f,0.f}, //-z
    };
    auto& draw_frame_context_ = frames_[frame_index_].frame_context;
    auto& scene = g_pSceneManager->GetSceneForRendering();
    if(scene.GetFirstLightNode())
    {
        int i = 0;
        int point_light_count = 0;
        for (auto& node : scene.LightNodes)
        {
            if (auto light = scene.GetLight(node.second->GetSceneObjectRef()); light != nullptr)
            {
                assert(i < kMaxLightNum - kMaxPointLightNum);
                Light single_light;
                single_light.light_color = light->GetColor().value_.xyz;
                single_light.light_direction = node.second->GetForwardDir();
                single_light.light_position = node.second->GetWorldPosition();
                single_light.light_instensity = light->GetIntensity() / 200000.f;
                single_light.falloff_begin = 800.f;
                single_light.falloff_end = 5000.f;
                Matrix4x4f light_view{};
                BuildViewMatrixLookToLH(light_view, node.second->GetWorldPosition(), -node.second->GetForwardDir(), Vector3f{ 0.f,1.f,0.f });
                single_light.vp_matrix_ = Transpose(light_view);
                switch (light->GetType())
                {
                case Engine::ELightType::kDirectional:
                {
                    BuildOrthographicMatrix(light_view, -640.f, 640.f, 450.f, -450.f, 10.f, 10000.f);
                    Transpose(light_view);
                    single_light.vp_matrix_ = light_view * single_light.vp_matrix_;
                    single_light.type = 0;
                    draw_frame_context_.lights_[i++] = single_light;
                }
                    break;
                case Engine::ELightType::kPoint:
                {
                    Matrix4x4f proj_mat_for_shaodw_map{};
                    Matrix4x4f view_mat_for_shaodw_map{};
                    BuildPerspectiveFovLHMatrix(proj_mat_for_shaodw_map,0.5f * kPi,1.f,10.f,10000.f);
                    Transpose(proj_mat_for_shaodw_map);
                    for(int j = 0; j < 6; ++j)
                    {
                        auto world_pos = node.second->GetWorldPosition();
                        BuildViewMatrixLookToLH(view_mat_for_shaodw_map, node.second->GetWorldPosition(),kForward[j],kUpDirs[j]);
                        draw_frame_context_.point_light_vp_mat[j + point_light_count * 6] = proj_mat_for_shaodw_map * Transpose(view_mat_for_shaodw_map);
                        view_mat_for_shaodw_map = {};
                    }
                    single_light.type = 1;
                    draw_frame_context_.lights_[kMaxLightNum - kMaxPointLightNum + point_light_count++] = single_light;
                }
                    break;
                case Engine::ELightType::kSpot:
                {
                    BuildPerspectiveFovLHMatrix(light_view, 1.57f, 16.f / 9.f, 10.f, 10000.f);
                    Transpose(light_view);
                    single_light.type = 2;
                    auto spot_light = std::dynamic_pointer_cast<SceneObjectSpotLight>(light);
                    single_light.inner_angle = spot_light->inner_angle_;
                    single_light.outer_angle = spot_light->outer_angle_;
                    single_light.vp_matrix_ = light_view * single_light.vp_matrix_;
                    draw_frame_context_.lights_[i++] = single_light;
                }
                    break;
                default:
                    break;
                }

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
