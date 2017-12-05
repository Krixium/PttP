#pragma once

#include <string>

#include <QtWidgets/QMainWindow>
#include <QScrollBar>
#include <QSerialPortInfo>
#include <QPlainTextEdit>

#include "FileManip.h"
#include "IOThread.h"
#include "ui_PttP.h"

using namespace std;

class PttP : public QMainWindow
{
	Q_OBJECT

public:
	PttP(QWidget *parent = Q_NULLPTR);

private:
	Ui::PttPClass ui;

	IOThread* mIOThread;

	void populatePortMenu();

public slots:
	void SetFileName(const string newFileName);

	void DisplayDataFromPort(const QString data);
};
