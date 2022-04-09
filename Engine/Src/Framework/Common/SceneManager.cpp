#include "pch.h"
#include "../Inc/Framework/Common/AssetLoader.h"
#include "../Inc/Framework/Parser/FbxParser.h"
#include "../Inc/Framework/Common/SceneManager.h"

Engine::SceneManager::~SceneManager()
{
}

int Engine::SceneManager::Initialize()
{
    int result = 0;
    p_scene_ = std::make_shared<Scene>();
    return result;
}

void Engine::SceneManager::Finalize()
{
}

void Engine::SceneManager::Tick()
{
}

int Engine::SceneManager::LoadScene(const char* scene_file_name)
{
    if (LoadFbxScene(scene_file_name)) {
        p_scene_->LoadResource();
        b_dirty_flag_ = true;
        b_has_scene_ = true;
        return 0;
    }
    return -1;
}

const Engine::Scene* Engine::SceneManager::GetSceneForRendering()
{
    return p_scene_.get();
}

bool Engine::SceneManager::IsSceneChanged()
{
    return false;
}

void Engine::SceneManager::NotifySceneIsRenderingQueued()
{
    b_dirty_flag_ = false;
}

void Engine::SceneManager::ResetScene()
{
    b_dirty_flag_ = true;
}

bool Engine::SceneManager::HasValidScene() const
{
    return b_has_scene_;
}

bool Engine::SceneManager::LoadFbxScene(const char* fbx_scene_file_name)
{
    FbxParser fbx_parser;
    p_scene_ = fbx_parser.Parse(fbx_scene_file_name);
    return p_scene_ != nullptr;
}
