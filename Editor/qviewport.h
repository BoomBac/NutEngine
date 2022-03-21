#pragma once

#include <QWidget>
namespace Ui { class qviewport; };

class qviewport : public QWidget
{
	Q_OBJECT

public:
	qviewport(QWidget *parent = Q_NULLPTR);
	~qviewport();

private:
	Ui::qviewport *ui;
};
