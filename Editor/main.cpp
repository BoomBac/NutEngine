#include "editor.h"
#include "stdafx.h"
#include "Inc/QTApplication.h"
#include "../Inc/RHI/D3D12GrahpicsManager.h"
#include "../Inc/Framework/Common/AssetLoader.h"
#include "../Inc/Framework/Common/MemoryManager.hpp"
#include "log.h"

namespace Engine
{
    GfxConfiguration config(8, 8, 8, 8, 32, 0, 0, 1280, 720, "NutEngine");
    IApplication* g_pApp = new QTApplication(config);
    GraphicsManager* g_pGfx = new D3d12GraphicsManager();
    SceneManager* g_pSceneManager = new SceneManager();
    AssetLoader* g_pAssetLoader = new AssetLoader();
    MemoryManager* g_pMemoryManager = new MemoryManager();
}
using Engine::g_pApp;
using Engine::g_pGfx;
using Engine::g_pMemoryManager;
using Engine::g_pAssetLoader;
using Engine::g_pSceneManager;

int main(int argc, char *argv[])
{
    g_pApp->Initialize();
    g_pMemoryManager->Initialize();
    g_pAssetLoader->Initialize();
    g_pAssetLoader->AddSearchPath("H:/Project_VS2019/NutEngine/Engine");

    auto p_main_window = dynamic_cast<QTApplication*>(g_pApp)->GetMainWindow();
    g_pSceneManager->LoadScene("box.fbx");
    g_pGfx->Initialize();
    while (p_main_window->isVisible())
    {
        g_pApp->Tick();
        g_pGfx->Clear();
        g_pGfx->Draw();
    }
    g_pApp->Finalize();
    g_pGfx->Finalize();

    g_pMemoryManager->Finalize();
    g_pAssetLoader->Finalize();
    
    return 0;
}
