#include "IOThread.h"
#include <QDebug>


const QByteArray IOThread::SYN = QByteArray(1, 0x16);
const QByteArray IOThread::STX = QByteArray(1, 0x02);
const QByteArray IOThread::ACK_FRAME = SYN + QByteArray(1, 0x06);
const QByteArray IOThread::ENQ_FRAME = SYN + QByteArray(1, 0x05);
const QByteArray IOThread::EOT_FRAME = SYN + QByteArray(1, 0x04);

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

	connect(mPort, &QSerialPort::readyRead, this, &IOThread::GetDataFromPort);
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

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: GetDataFromPort()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: void GetDataFromPort (void)
--
-- RETURNS: void.
--
-- NOTES:
--
-- A Qt Slot.
--
-- This slot is triggered whenever the serial port has something to read. Logic is handled by
-- the protocol design.
-------------------------------------------------------------------------------------------------*/
void IOThread::GetDataFromPort()
{
	QByteArray buffer = mPort->readAll();
	qDebug() << "Buffer = " << buffer;

	if (buffer == ENQ_FRAME)
	{
		qDebug() << "buffer was ENQ";
		SendACK();
	}

	if (buffer == ACK_FRAME)
	{
		qDebug() << "buffer was ACK";
		emit LineReadyToSend();
	}

	emit DataFrameRecieved(buffer);
}


/*-------------------------------------------------------------------------------------------------
-- FUNCTION: run()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: void run (void)
--
-- RETURNS: void.
--
-- NOTES:
-- The entry point for QThread.
-------------------------------------------------------------------------------------------------*/
void IOThread::run()
{
	while (mRunning) 
	{
		
	}
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: run()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: void run (void)
--
-- RETURNS: void.
--
-- NOTES:
-- The entry point for QThread.
-------------------------------------------------------------------------------------------------*/
void IOThread::Send(const QByteArray data)
{
	qDebug() << "Sending data";
	mPort->write(data);
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: SendACK()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: void SendACK (void)
--
-- RETURNS: void.
--
-- NOTES:
-- Sends a single ACk through the serial port.
-------------------------------------------------------------------------------------------------*/
void IOThread::SendACK()
{
	qDebug() << "Sending ACK";
	mPort->write(ACK_FRAME);
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: SendENQ()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: void SendENQ (void)
--
-- RETURNS: void.
--
-- NOTES:
-- Sends a single ENQ through the serial port.
-------------------------------------------------------------------------------------------------*/
void IOThread::SendENQ()
{
	qDebug() << "Sending ENQ";
	mPort->write(ENQ_FRAME);
}