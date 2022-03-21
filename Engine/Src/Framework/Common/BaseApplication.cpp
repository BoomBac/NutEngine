#include "pch.h"

#include "Framework/Common/BaseApplication.h"

Engine::BaseApplication::BaseApplication(GfxConfiguration& gcf) : config_(gcf){}

int Engine::BaseApplication::Initialize()
{
	int ret = 0;
	std::cout << config_.GetInfo() << std::endl;
	CreateMainWindow();
	return ret;
}

void Engine::BaseApplication::Finalize()
{
}

void Engine::BaseApplication::Tick()
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
