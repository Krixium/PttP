#include "IOThread.h"

const QByteArray IOThread::ACK = QByteArray(1, 0x06);
const QByteArray IOThread::ENQ = QByteArray(1, 0x05);

IOThread::IOThread(QObject *parent)
	: QThread(parent)
	, mRunning(true)
	, mPort(new QSerialPort("COM1", this))
{
	mPort->setBaudRate(QSerialPort::Baud9600);
	mPort->setDataBits(QSerialPort::Data8);
	mPort->setParity(QSerialPort::NoParity);
	mPort->setStopBits(QSerialPort::OneStop);
	mPort->setFlowControl(QSerialPort::NoFlowControl);
	mPort->open(QSerialPort::ReadWrite);
}

IOThread::~IOThread() 
{
	mRunning = false;
	mPort->close();
}

QSerialPort* IOThread::GetPort()
{
	return mPort;
}

QString IOThread::GetDataFromPort()
{
	QByteArray buffer = mPort->readAll();
	
	if (buffer == IOThread::ACK)
	{
		return "Be Bada Bap";
	}

	return "";
}

void IOThread::run()
{
	while (mRunning) 
	{
		// Threading code here
	}
}

void IOThread::SendACK()
{
	mPort->write(ACK);
}

void IOThread::SendENQ()
{
	mPort->write(ENQ);
}