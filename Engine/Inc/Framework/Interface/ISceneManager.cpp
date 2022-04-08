#include "pch.h"
#include "../Inc/Framework/Common/AssetLoader.h"
#include "../Inc/Framework/Parser/FbxParser.h"
#include "ISceneManager.h"

Engine::ISceneManager::~ISceneManager()
{
}

int Engine::ISceneManager::Initialize()
{
    int result = 0;
    return result;
}

void Engine::ISceneManager::Finalize()
{
}

void Engine::ISceneManager::Tick()
{
}

void Engine::ISceneManager::LoadScene(const char* scene_file_name)
{
    LoadFbxScene(scene_file_name);
}

const Engine::Scene& Engine::ISceneManager::GetSceneForRendering()
{
    return *p_scene_;
}

void Engine::ISceneManager::LoadFbxScene(const char* fbx_scene_file_name)
{
    FbxParser fbx_parser;
    p_scene_ = fbx_parser.Parse(fbx_scene_file_name);
}
