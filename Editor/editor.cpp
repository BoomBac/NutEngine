#include "editor.h"
#include "stdafx.h"

#include "qviewport.h"
#include "log.h"
#include "Framework/Common/InputManager.h"

#include <QComboBox>

using Engine::g_InputManager;
using Engine::InputManager;

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

void Editor::keyPressEvent(QKeyEvent* ev)
{
    switch (ev->key())
    {
    case Qt::Key::Key_W:
    {
        g_InputManager->ButtonDown(Engine::EKeyButton::kW);
        Logger::Log("w press");
    }       
        break;
    case Qt::Key::Key_A:
    {
        g_InputManager->ButtonDown(Engine::EKeyButton::kA);
        Logger::Log("a press");
    }
        break;
    case Qt::Key::Key_S:
    {
        g_InputManager->ButtonDown(Engine::EKeyButton::kS);
        Logger::Log("s press");
    }
        break;
    case Qt::Key::Key_D:
    {
        g_InputManager->ButtonDown(Engine::EKeyButton::kD);
        Logger::Log("d press");
    }
        break;
    default:
            break;
    }
}

void Editor::keyReleaseEvent(QKeyEvent* ev)
{
    switch (ev->key())
    {
    case Qt::Key::Key_W:
    {
        g_InputManager->ButtonUp(Engine::EKeyButton::kW);
        Logger::Log("w release");
    }
    break;
    case Qt::Key::Key_A:
    {
        g_InputManager->ButtonUp(Engine::EKeyButton::kA);
        Logger::Log("a release");
    }
    break;
    case Qt::Key::Key_S:
    {
        g_InputManager->ButtonUp(Engine::EKeyButton::kS);
        Logger::Log("s release");
    }
    break;
    case Qt::Key::Key_D:
    {
        g_InputManager->ButtonUp(Engine::EKeyButton::kD);
        Logger::Log("d release");
    }
    break;
    default:
        break;
    }
}

void Editor::mouseMoveEvent(QMouseEvent* ev)
{
    static Engine::MouseMoveEvent mev{};
    mev.x_ = ev->pos().x();
    mev.y_ = ev->pos().y();
    g_InputManager->MouseMove(mev);
}

void Editor::mousePressEvent(QMouseEvent* event)
{
    g_InputManager->SetPreMousePos(event->pos().x(), event->pos().y());
}

void Editor::mouseReleaseEvent(QMouseEvent* event)
{
}
