#include "editor.h"
#include "stdafx.h"

#include "qviewport.h"
#include "log.h"

#include <QComboBox>


Editor::Editor(QWidget *parent) : QMainWindow(parent)
{
    ui.setupUi(this);
    p_viewport_ = new qviewport(this);
    p_viewport_->setFixedSize(QSize(1280, 720));
    Init();
    p_viewport_->show();
    handle_vp_ = (HWND)p_viewport_->winId();
}

HWND Editor::GetViewportHandle() const
{
    assert(p_viewport_ != nullptr);
    return handle_vp_;
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
