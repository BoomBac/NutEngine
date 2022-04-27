#include "pch.h"
#include "Framework/Common/BaseApplication.h"
#include "Framework/Common/TimerManager.h"
#include "Framework/Common/Log.h"

Engine::BaseApplication::BaseApplication(GfxConfiguration& gcf) : config_(gcf)
{
	
}
int Engine::BaseApplication::Initialize()
{
	int ret = 0;
	CreateMainWindow();
	NE_LOG(ALL,kNormal,"Initialize MemoryManager")
	if(ret += g_pMemoryManager->Initialize() != 0)	
	{
		NE_LOG(ALL, kError, "Initialize MemoryManager failed!")
		return ret;
	}
	NE_LOG(ALL, kNormal, "Initialize AssetManager")
	if (ret += g_pAssetLoader->Initialize() != 0)
	{
		NE_LOG(ALL, kError, "Initialize AssetManager failed!")
			return ret;
	}
	NE_LOG(ALL, kNormal, "Initialize SceneManager")
	if (ret += g_pSceneManager->Initialize() != 0)
	{
		NE_LOG(ALL, kError, "Initialize SceneManager failed!")
			return ret;
	}
	NE_LOG(ALL, kNormal, "Initialize GraphicsManager")
	if (ret += g_pGraphicsManager->Initialize() != 0)
	{
		NE_LOG(ALL, kError, "Initialize GraphicsManager failed!")
			return ret;
	}
	NE_LOG(ALL, kNormal, "Initialize InputManager")
	if (ret += g_InputManager->Initialize() != 0)
	{
		NE_LOG(ALL, kError, "Initialize InputManager failed!")
			return ret;
	}
	NE_LOG(ALL, kNormal, "Initialize PhysicsManager")
	if (ret += g_pPhysicsManager->Initialize() != 0)
	{
		NE_LOG(ALL, kError, "Initialize PhysicsManager failed!")
			return ret;
	}
	NE_LOG(ALL, kNormal, "Initialize TimerManager")
	if (ret += g_pTimerManager->Initialize() != 0)
	{
		NE_LOG(ALL, kError, "Initialize TimerManager failed!")
			return ret;
	}
	return ret;
}

void Engine::BaseApplication::Finalize()
{
	g_InputManager->Finalize();
	g_pGraphicsManager->Finalize();
	g_pPhysicsManager->Finalize();
	g_pSceneManager->Finalize();
	g_pAssetLoader->Finalize();
	g_pMemoryManager->Finalize();
}

void Engine::BaseApplication::Tick()
{
	g_pMemoryManager->Tick();
	g_pAssetLoader->Tick();
	g_pSceneManager->Tick();
	g_InputManager->Tick();
	g_pPhysicsManager->Tick();
	g_pGraphicsManager->Tick();
}

void Engine::BaseApplication::OnDraw()
{
}

void Engine::BaseApplication::SetCommandLineParameters(int argc, char** argv)
{
	argc_ = argc;
	pp_argv_ = argv;
}

int Engine::BaseApplication::GetCommandLineArgumentsCount() const
{
	return argc_;
}

const char* Engine::BaseApplication::GetCommandLineArgument(int index) const
{
	assert(index < argc_);
	return pp_argv_[index];
}

bool Engine::BaseApplication::IsQuit() const
{
	return sb_quit_;
}

void Engine::BaseApplication::CreateMainWindow()
{
}
