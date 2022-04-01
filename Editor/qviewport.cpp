#include "qviewport.h"
#include "ui_qviewport.h"
#include <QTimer>

#include "Engine.h"

qviewport::qviewport(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::qviewport();
	ui->setupUi(this);
	setFixedSize(1280, 720);
	this->setAttribute(Qt::WA_PaintOnScreen);
	setFocusPolicy(Qt::FocusPolicy::ClickFocus);
	//HWND d = (HWND)winId();
	//Engine::InitPipeline(d);
	//Engine::LoadAsset();
	//QTimer* timer = new QTimer(this);
	//timer->start(16.7);
	//connect(timer, &QTimer::timeout, []() {
	//	Engine::OnRender();
	//	});
}

qviewport::~qviewport()
{
	delete ui;
}
