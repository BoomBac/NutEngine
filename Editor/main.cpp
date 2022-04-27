#include "editor.h"
#include "stdafx.h"
#include "Inc/QTApplication.h"
#include "RHI/D3D12GrahpicsManager.h"
#include "Framework/Common/AssetLoader.h"
#include "Framework/Common/MemoryManager.hpp"
#include "Framework/Common/InputManager.h"
#include "Framework/Common/Log.h"
#include "Framework/Common/Global.h"
#include "Framework/Common/TimerManager.h"
#include "Physics/NutPhysicsManager.h"

#include "Game/Empty.h"

namespace Engine
{
    GfxConfiguration config(8, 8, 8, 8, 32, 0, 0, 1280, 720, "NutEngine");
    IApplication* g_pApp = new QTApplication(config);
    GraphicsManager* g_pGraphicsManager = new D3d12GraphicsManager();
    SceneManager* g_pSceneManager = new SceneManager();
    AssetLoader* g_pAssetLoader = new AssetLoader();
    MemoryManager* g_pMemoryManager = new MemoryManager();
    InputManager* g_InputManager = new InputManager();
    LogManager* g_pLogManager = new LogManager();
    TimerManager* g_pTimerManager = new TimerManager();
    IPhysicsManager* g_pPhysicsManager = new NutPhysicsManager();
    GameLogic* g_pGameLogic = new Empty();
}
using namespace Engine;
using Engine::g_pApp;
using Engine::g_pGraphicsManager;
using Engine::g_pMemoryManager;
using Engine::g_pAssetLoader;
using Engine::g_pSceneManager;
using Engine::g_InputManager;
using Engine::g_pLogManager;
using Engine::g_pTimerManager;


int main(int argc, char *argv[])
{
    Engine::LoadConfigFile("H:/Project_VS2019/NutEngine/Engine/config.ini");
    int error = 0;
    error += g_pLogManager->Initialize();
    error += g_pApp->Initialize();
    g_pAssetLoader->AddSearchPath("H:/Project_VS2019/NutEngine/Engine");
    g_pSceneManager->LoadScene();
    assert(error==0);
    NE_LOG(ALL, kWarning, "Editor launch")
    g_pTimerManager->Reset();
    auto p_main_window = dynamic_cast<QTApplication*>(g_pApp)->GetMainWindow();
    while (p_main_window->isVisible())
    {
        g_pApp->Tick();
    }
    g_pApp->Finalize();
    return 0;
}
