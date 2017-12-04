#include "IOThread.h"

#include <QDebug>

const QByteArray IOThread::SYN_BYTE = QByteArray(1, SYN);
const QByteArray IOThread::STX_BYTE = QByteArray(1, STX);
const QByteArray IOThread::ACK_FRAME = SYN_BYTE + QByteArray(1, ACK);
const QByteArray IOThread::ENQ_FRAME = SYN_BYTE + QByteArray(1, ENQ);
const QByteArray IOThread::EOT_FRAME = SYN_BYTE + QByteArray(1, EOT);

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

	connect(mPort, &QSerialPort::readyRead, this, &IOThread::GetDataFromPort);

	mBuffer = QByteArray();

	/*
	QByteArray input = QByteArray("123456789");
	QByteArray frame = makeFrame(input);
	bool isFrameValid = isDataFrameValid(frame);
	string data = getDataFromFrame(frame);
	*/
}

IOThread::~IOThread()
{
	mRunning = false;
	mPort->close();
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: GetPort()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang
--
-- INTERFACE: QSerialPort* GetPort (void)
--
-- RETURNS: A pointer to the QSerialPort.
--
-- NOTES:
-- Getter function for the pointer to the programs QSerialPort.
-------------------------------------------------------------------------------------------------*/
QSerialPort* IOThread::GetPort()
{
	return mPort;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: SetPort()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang
--
-- INTERFACE: void SetPort (void)
--
-- NOTES:
-- A Qt slot.
--
-- This function is triggered when a QAction representing a serial port is pressed. This function
-- will grab the name of the port from that action and set the port to it.
-------------------------------------------------------------------------------------------------*/
void IOThread::SetPort()
{
	mPort->close();
	mPort->setPortName(((QAction*)QObject::sender())->text());
	mPort->open(QSerialPort::ReadWrite);
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
-- When there is data to read in the ports it is read into a buffer and a checkBuffer() is called
-- to handle it.
-------------------------------------------------------------------------------------------------*/
void IOThread::GetDataFromPort()
{
	mBuffer += mPort->readAll();
	handleBuffer();
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: handleBuffer()
--
-- DATE: December 4, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang
--
-- INTERFACE: void handleBuffer (void)
--
-- RETURNS: void.
--
-- NOTES:
--
-- When new data is read in from the buffer, this function will check the buffer to see if it
-- contains a valid frame and then handles it accordingly.
-------------------------------------------------------------------------------------------------*/
void IOThread::handleBuffer()
{
	qDebug() << mBuffer;

	if (mBuffer.contains(ENQ_FRAME))
	{
		qDebug() << "ENQ received";
		SendACK();
		mBuffer.clear();
	}

	if (mBuffer.contains(ACK_FRAME))
	{
		qDebug() << "ACK received";
		emit LineReadyToSend();
		mBuffer.clear();
	}

	// This might have to be more robust later down the line
	if (mBuffer.contains(SYN_BYTE + STX_BYTE))
	{
		int dataFrameStart = mBuffer.indexOf(SYN_BYTE);
		QByteArray dataFrame = mBuffer.mid(dataFrameStart, 516);
		if (isDataFrameValid(dataFrame))
		{
			qDebug() << "Data frame is good";
			emit DataReceieved(getDataFromFrame(dataFrame));
			SendACK();
			mBuffer.clear();
		}
		else
		{
			qDebug() << "Error detected in data frame";
		}
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
void IOThread::run()
{
	while (mRunning)
	{

	}
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: Send()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang
--
-- INTERFACE: void Send (const QByteArray& data)
--		const QByteArray& data: The data to send through the serial port.
--
-- RETURNS: void.
--
-- NOTES:
-- Processes data as nessecary in a non-destructive fashion and sends it over the serial port.
-------------------------------------------------------------------------------------------------*/
void IOThread::Send(const QByteArray& data)
{
	qDebug() << "Sending data";
	mPort->write(makeFrame(data));
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

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: makeFrame()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang
--
-- INTERFACE: QByteArray makeFrame (const QByteArray& data)
--		const QByteArray& data: The data to frame.
--
-- RETURNS: The data wrapped in a frame. 
--
-- NOTES:
-- Wraps the given data in a frame specified by the Power to the Protocoleriat protocol.
--
-- The checksum used is Qt::ChecksumIso3309(CRC-16/X-25)
--		Poly:	0x1021
--		Init:	0xFFFF
--		RefIn:	True
--		RefOut:	True
--		XorOut:	0xFFFF
-------------------------------------------------------------------------------------------------*/
QByteArray IOThread::makeFrame(const QByteArray& data)
{
	QByteArray stuffedData = QByteArray(512 - data.size(), 0x0);
	stuffedData.prepend(data);
	QByteArray frame = SYN_BYTE + STX_BYTE + stuffedData;
	quint16 checksum = qChecksum(stuffedData, stuffedData.size(), Qt::ChecksumIso3309);
	frame = frame << checksum;
	return frame;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: isDataFrameValid()
--
-- DATE: November 30, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang
--
-- INTERFACE: bool isDataFrameValid (const QByteArray& frame)
--		const QByteArray& frame: The frame recieved from the serial port.
--
-- RETURNS: True if no errors were detected by the CRC. 
--
-- NOTES:
-- Tries to detect if any errors occured on the data in the frame and returns true if if no errors
-- are deteted.
-------------------------------------------------------------------------------------------------*/
bool IOThread::isDataFrameValid(const QByteArray& frame)
{
	if (frame.size() != 516) return false;
	QByteArray data = frame.mid(2, 512);
	QByteArray crc = frame.mid(514);
	QByteArray checksum = QByteArray();  
	checksum = checksum << qChecksum(data, data.size(), Qt::ChecksumIso3309);
	return checksum == crc;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: getDataFromFrame()
--
-- DATE: November 30, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang
--
-- INTERFACE: string getDataFromFrame (const QByteArray& frame)
--		const QByteArray& frame: The frame recieved from the serial port.
--
-- RETURNS: The data contained in the frame.
--
-- NOTES:
-- Unwraps the data from the frame and returns it as a stirng.
-------------------------------------------------------------------------------------------------*/
string IOThread::getDataFromFrame(const QByteArray& frame)
{
	return frame.mid(2, 512).toStdString();
}

