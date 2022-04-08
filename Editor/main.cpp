#include "editor.h"
#include "stdafx.h"
#include "Inc/QTApplication.h"
#include "../Inc/RHI/D3D12GrahpicsManager.h"
//#include "../Inc/Framework/Parser/FbxParser.h"

namespace Engine
{
    IApplication* g_pApp = new QTApplication();
    GraphicsManager* g_pGfx = new D3d12GraphicsManager();
}
using Engine::g_pApp;
//using Engine::g_pGfx;
//using Engine::g_pMemoryManager;
//using Engine::g_pAssetLoader;

int main(int argc, char *argv[])
{
    g_pApp->Initialize();
    //g_pMemoryManager->Initialize();
    //g_pAssetLoader->Initialize();
  //  g_pGfx->Initialize();
    auto p_main_window = dynamic_cast<QTApplication*>(g_pApp)->GetMainWindow();
    while (p_main_window->isVisible())
    {
        g_pApp->Tick();
    }
    g_pApp->Finalize();
    //g_pMemoryManager->Finalize();
    //g_pAssetLoader->Finalize();
   // g_pGfx->Finalize();
    return 0;
}
