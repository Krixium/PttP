#pragma once

#include <QFileDialog>
#include <QtWidgets/QMainWindow>

#include "ui_PttP.h"

class PttP : public QMainWindow
{
	Q_OBJECT

public:
	PttP(QWidget *parent = Q_NULLPTR);

private:
	Ui::PttPClass ui;
	QString mSelectedFileName;

private slots:
	void selectFile();
};
