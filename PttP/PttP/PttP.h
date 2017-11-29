#pragma once

#include <string>

#include <QtWidgets/QMainWindow>

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

	FileManip* mFile;
	IOThread* mIOThread;

public slots:
	void SetFileName(const string newFileName);

	void SendBytesOverPort();
};
