#include "IOThread.h"

const QByteArray IOThread::ACK = QByteArray(1, 0x06);
const QByteArray IOThread::ENQ = QByteArray(1, 0x05);

IOThread::IOThread(QObject *parent)
	: QThread(parent)
	, mPort(new QSerialPort("COM1", this))
{
	mPort->setBaudRate(QSerialPort::Baud9600);
	mPort->setDataBits(QSerialPort::Data8);
	mPort->setParity(QSerialPort::NoParity);
	mPort->setStopBits(QSerialPort::OneStop);
	mPort->setFlowControl(QSerialPort::NoFlowControl);
}

void IOThread::SendACK()
{
	mPort->open(QSerialPort::WriteOnly);
	mPort->write(ACK);
	mPort->close();
}

void IOThread::SendENQ()
{
	mPort->open(QSerialPort::WriteOnly);
	mPort->write(ENQ);
	mPort->close();
}