#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_editor.h"


class Editor : public QMainWindow
{
    Q_OBJECT

public:
    Editor(QWidget* parent = Q_NULLPTR);
    HWND GetViewportHandle() const;
private:
    HWND handle_vp_;
    Ui::EditorClass ui;
    void Init();
    QWidget* p_viewport_;
};

