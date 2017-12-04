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
	, mPort(new QSerialPort(this))
	, mFile(new FileManip(this))
	, mTxFrameCount(0)
{
	mPort->setBaudRate(QSerialPort::Baud9600);
	mPort->setDataBits(QSerialPort::Data8);
	mPort->setParity(QSerialPort::NoParity);
	mPort->setStopBits(QSerialPort::OneStop);
	mPort->setFlowControl(QSerialPort::NoFlowControl);

	connect(mPort, &QSerialPort::readyRead, this, &IOThread::GetDataFromPort);

	mBuffer = QByteArray();
}

IOThread::~IOThread()
{
	mRunning = false;
	mPort->close();
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
	qDebug() << "Buffer:" << mBuffer;
	if (mBuffer.contains(ENQ_FRAME))
	{
		qDebug() << "ENQ received";
		SendACK();
		mBuffer.clear();
	}

	if (mBuffer.contains(ACK_FRAME))
	{
		if (mTxFrameCount < 10)
		{
			qDebug() << "ACK received";
			sendBytes();
			mBuffer.clear();
			mTxFrameCount++;
		}
		else
		{
			SendEOT();
			mTxFrameCount = 0;
		}
	}

	if (mBuffer.contains(EOT_FRAME))
	{
		qDebug() << "EOT received.";
	}

	if (mBuffer.contains(SYN_BYTE + STX_BYTE))
	{
		int dataFrameStart = mBuffer.indexOf(SYN_BYTE);
		QByteArray dataFrame = mBuffer.mid(dataFrameStart, DATA_FRAME_SIZE);

		if (isDataFrameValid(dataFrame))
		{
			qDebug() << "Data frame is good";
			emit DataReceieved(getDataFromFrame(dataFrame));
			SendACK();
			mBuffer.clear();
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
-- INTERFACE: void Send ()
--
-- RETURNS: void.
--
-- NOTES:
-- Processes data as nessecary in a non-destructive fashion and sends it over the serial port.
-------------------------------------------------------------------------------------------------*/
void IOThread::sendBytes()
{
	qDebug() << "Sending data";
	QByteArray data = mFile->GetNextBytes();
	QByteArray frame = makeFrame(data);
	mPort->write(frame);
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
-- FUNCTION: SendEOT()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang
--
-- INTERFACE: void SendEOT (void)
--
-- RETURNS: void.
--
-- NOTES:
-- Sends a single EOT through the serial port.
-------------------------------------------------------------------------------------------------*/
void IOThread::SendEOT()
{
	qDebug() << "Sending EOT";
	mPort->write(EOT_FRAME);
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
-------------------------------------------------------------------------------------------------*/
QByteArray IOThread::makeFrame(const QByteArray& data)
{
	QByteArray stuffedData = QByteArray(DATA_LENGTH - data.size(), 0x0);
	stuffedData.prepend(data);
	uint32_t crc = CRC::Calculate(stuffedData.data(), DATA_LENGTH, CRC::CRC_32());

	QByteArray frame = SYN_BYTE + STX_BYTE + stuffedData;
	frame = frame << crc;

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
	// Check size (518 bytes)
	if (frame.size() != DATA_FRAME_SIZE) return false;

	// Grab data (512 bytes)
	QByteArray frameData = frame.mid(2, DATA_LENGTH);

	// Grab crc (4 bytes)
	QByteArray receivedCrc = frame.mid(DATA_HEADER_SIZE + DATA_LENGTH);

	QByteArray recalculatedCrc = QByteArray();
	recalculatedCrc = recalculatedCrc << CRC::Calculate(frameData.data(), DATA_LENGTH, CRC::CRC_32());

	return recalculatedCrc == receivedCrc;
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
	return frame.mid(DATA_HEADER_SIZE, DATA_LENGTH).toStdString();
}

