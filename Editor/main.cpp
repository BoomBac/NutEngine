#include "editor.h"
#include "stdafx.h"
#include "Inc/QTApplication.h"
#include "Inc/RHI/D3D12GrahpicsManager.h"

#include "Engine.h"


using Engine::g_pApp;

namespace Engine
{
    IApplication* g_pApp = new QTApplication();
    GraphicsManager* g_pGfx = new D3d12GraphicsManager();
}

int main(int argc, char *argv[])
{
    g_pApp->Initialize();

    Engine::g_pGfx->Initialize();

    auto p_main_window = dynamic_cast<QTApplication*>(g_pApp)->GetMainWindow();
    while (p_main_window->isVisible())
    {
        g_pApp->Tick();
    }
    g_pApp->Finalize();
    return 0;
}
