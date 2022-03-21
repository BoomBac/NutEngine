#pragma once
#include "../Interface/IApplication.h"
#include "GfxConfiguration.h"

namespace Engine
{
	class BaseApplication : public IApplication
	{
	public:
		BaseApplication() = delete;
		explicit BaseApplication(GfxConfiguration& gcf);
		int Initialize() override;
		void Finalize() override;
		void Tick() override;

		void SetCommandLineParameters(int argc, char** argv) override;
		[[nodiscard]] int GetCommandLineArgumentsCount() const override;
		[[nodiscard]] const char* GetCommandLineArgument(int index) const override;

		[[nodiscard]] bool IsQuit() const override;
		void RequestQuit() override { sb_quit_ = true; }
		[[nodiscard]] inline const GfxConfiguration& GetConfiguration()const override {return config_;}
	protected:
		void CreateMainWindow() override;

		// Flag if need quit the main loop of the application
		static inline bool sb_quit_ = false;
		GfxConfiguration config_;
		int argc_;
		char** pp_argv_;
	};
}//namespace Engine