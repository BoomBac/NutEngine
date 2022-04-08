#include "qviewport.h"
#include "ui_qviewport.h"


qviewport::qviewport(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::qviewport();
	ui->setupUi(this);
	setFixedSize(1280, 720);
	this->setAttribute(Qt::WA_PaintOnScreen);
	setFocusPolicy(Qt::FocusPolicy::ClickFocus);
}

qviewport::~qviewport()
{
	delete ui;
}
