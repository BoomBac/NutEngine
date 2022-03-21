#include "editor.h"
#include "stdafx.h"
#include "log.h"

#include <QComboBox>

//#pragma comment(lib,"Engine.lib")
using Logger::Log;

Editor::Editor(QWidget *parent) : QMainWindow(parent)
{
    ui.setupUi(this);
    Init();
	//Log(ELogLevel::kLog, ELogGroup::kLogTemp, "hello{}", 20);
	//for (int i = 0; i < 20; i++)
	//	Log(ELogLevel::kError, ELogGroup::kLogTemp, "hello{}", i);
	//for (int i = 20; i < 40; i++)
	//	Log("LogTest{}", i);
}

void Editor::Init()
{
    Logger::InitializeLogger(ui.tb_log,ui.cb_log_level,ui.cb_log_group);
    
    ui.centralWidget->setFixedWidth(1600);
    ui.centralWidget->setFixedHeight(900);
    connect(ui.cb_log_level, &QComboBox::currentTextChanged,
        [this](QString text)
        {
            Logger::Filter(NutEnum::ToELogGroup(ui.cb_log_group->currentText()), NutEnum::ToELogLevel(text));
        }
    );
    connect(ui.cb_log_group, &QComboBox::currentTextChanged,
        [this](QString text)
        {
            Logger::Filter(NutEnum::ToELogGroup(text), NutEnum::ToELogLevel(ui.cb_log_level->currentText()));
        }
    );
}
