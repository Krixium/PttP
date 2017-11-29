#include "PttP.h"

using namespace std;

PttP::PttP(QWidget *parent)
	: QMainWindow(parent)
	, mIOThread(new IOThread(this))
	, mFile(new FileManip(this))
{
	ui.setupUi(this);

	connect(ui.pushButtonSelect, &QPushButton::pressed, mFile, &FileManip::SelectFile);
	connect(mFile, &FileManip::fileChanged, this, &PttP::SetFileName);

	connect(mIOThread->GetPort(), &QSerialPort::readyRead, this, &PttP::ReadFromPort);

	connect(mIOThread, &IOThread::LineReadyToSend, this, &PttP::SendBytesOverPort);

	mIOThread->start();
}

void PttP::ReadFromPort()
{
	ui.plainTextEdit->insertPlainText(mIOThread->GetDataFromPort());
}

void PttP::SetFileName(string newFileName)
{
	ui.labelSelectedFile->setText(QString(newFileName.c_str()));
}

void PttP::SendBytesOverPort()
{
	mIOThread->Send(mFile->GetNextBytes());
}