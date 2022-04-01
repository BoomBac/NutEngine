#pragma once
#include <QtWidgets/QApplication>

#include "editor.h"
#include "Inc/Framework/Common/BaseApplication.h"

using Engine::BaseApplication;
using Engine::GfxConfiguration;

static GfxConfiguration qt_config;


class QTApplication : public BaseApplication
{
public:
	QTApplication();
	int Initialize() override;
	void Finalize() override;
	// One cycle of the main loop
	void Tick() override;
	void* GetMainWindowHandler() override;
	QMainWindow* GetMainWindow();
	QApplication* GetApp();
	~QTApplication();	
private:
	QApplication* p_app_;
	QMainWindow* p_editor_;
	HWND handle_;
};