#ifndef __BASE_APPLICATION_H__
#define __BASE_APPLICATION_H__
#include "../Interface/IApplication.h"
#include "GraphicsManager.h"
#include "MemoryManager.hpp"
#include "AssetLoader.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "../Interface/IPhysicsManager.h"
#include "GameLogic.h"

#include "GfxConfiguration.h"

namespace Engine
{
	class BaseApplication : public IApplication
	{
	public:
		BaseApplication() = delete;
		explicit BaseApplication(GfxConfiguration& gcf);
		BaseApplication& operator=(BaseApplication& other) = delete;
		BaseApplication(BaseApplication& other) = delete;
		int Initialize() override;
		void Finalize() override;
		void Tick() override;
		void OnDraw() override;
		void SetCommandLineParameters(int argc, char** argv) override;
		[[nodiscard]] int GetCommandLineArgumentsCount() const override;
		[[nodiscard]] const char* GetCommandLineArgument(int index) const override;

		[[nodiscard]] bool IsQuit() const override;
		void RequestQuit() override { sb_quit_ = true; }
		[[nodiscard]] inline const GfxConfiguration& GetConfiguration()const override { return config_; }
	protected:
		void CreateMainWindow() override;

		// Flag if need quit the main loop of the application
		static inline bool sb_quit_ = false;
		GfxConfiguration config_;
		int argc_;
		char** pp_argv_;
	};
}//namespace Engine
#endif // !__BASE_APPLICATION_H__

