/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: IOThread.cpp - The thread class that handles all IO of the program.
--
-- PROGRAM: PttP
--
-- FUNCTIONS:
-- IOThread()
-- ~IOThread()
-- void run()
-- 
-- void setFlag(const uint32_t flag, const bool state)
-- bool isFlagSet(const uint32_t flag)
--
-- void startTimeout(const int ms)
-- void updateTimeout()
--
-- void sendACK()
-- void sendENQ()
-- void sendEOT()
-- void sendRVI()
-- void sendFrame()
-- void resendFrame()
-- void resetFlags()
-- void resetFlagsNoTimeout()
-- void backoff()
-- QByteArray makeFrame(const QByteArray& data)
--
-- void handleBuffer()
-- void checkPotentialDataFrame()
--
-- bool isDataFrameValid(const QByteArray& frame)
-- QString getDataFromFrame(const QByteArray& frame)
--
-- void SetRVI()
-- void SendFile()
-- void GetDataFromPort()
-- void SetPort()
-- void writeToPort(const QByteArray& frame)
--
-- DATE: Nov 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER: Benny Wang, Delan Elliot, Roger Zhang
--
-- NOTES:
-- This class handles all IO of the program.
-- The task performed by this program depends on the state that it is in. The states of the program are determined by
-- a set of bit flags. The task that are performed as specificed by the Power to the Protocoleriat protocol.
----------------------------------------------------------------------------------------------------------------------*/
#include "IOThread.h"

#include <QDebug>

const QByteArray IOThread::SYN_BYTE = QByteArray(1, SYN);
const QByteArray IOThread::STX_BYTE = QByteArray(1, STX);
const QByteArray IOThread::ACK_FRAME = SYN_BYTE + QByteArray(1, ACK);
const QByteArray IOThread::ENQ_FRAME = SYN_BYTE + QByteArray(1, ENQ);
const QByteArray IOThread::EOT_FRAME = SYN_BYTE + QByteArray(1, EOT);
const QByteArray IOThread::RVI_FRAME = SYN_BYTE + QByteArray(1, RVI);

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: IOThread
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang
--
-- PROGRAMMER:		Benny Wang
--
-- INTERFACE:		IOThread (QObject* parent)
--						QObject* parent: The parent QObject.
--
-- RETURNS:			void.
--
-- NOTES:
-- Constructor for IOThread.
--
-- Sets all flags and buffers to their default state.
-- Creates all Qt signal slot connetions that are required.
----------------------------------------------------------------------------------------------------------------------*/
IOThread::IOThread(QObject *parent)
	: QThread(parent)
	, mRunning(true)
	, mFlags(0)
	, mPort(new QSerialPort(this))
	, mFile(new FileManip(this))
	, mTxFrameCount(0)
	, mRTXCount(0)
	, byteError(0)
	, byteValid(1)
{
	mPort->setBaudRate(QSerialPort::Baud9600);
	mPort->setDataBits(QSerialPort::Data8);
	mPort->setParity(QSerialPort::NoParity);
	mPort->setStopBits(QSerialPort::OneStop);
	mPort->setFlowControl(QSerialPort::NoFlowControl);

	connect(mPort, &QSerialPort::readyRead, this, &IOThread::GetDataFromPort, Qt::QueuedConnection);
	connect(this, &IOThread::writeToPortSignal, this, &IOThread::writeToPort, Qt::QueuedConnection);

	mBuffer = QByteArray();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ~IOThread
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang
--
-- PROGRAMMER:		Benny Wang
--
-- INTERFACE:		~IOThread(void)
--
-- RETURNS:			void.
--
-- NOTES:
-- Deconstructor for IOThread.
--
-- Stops the running loop, closes the serial port, and waits for the thread to stop. If the thread doesn't stop the
-- deconstructor will forcefully terminate it.
----------------------------------------------------------------------------------------------------------------------*/
IOThread::~IOThread()
{
	mRunning = false;
	mPort->close();
	quit();
	if (!wait(3000))
	{
		terminate();
		wait();
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: SetPort
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wan
--
-- PROGRAMMER:		Benny Wang
--
-- INTERFACE:		void SetPort(void)	
--
-- RETURNS:			void.
--
-- NOTES:
-- This is a Qt slot.
--
-- When a port is selected, this function will get the text of the calling QAction which is the name of the port. It
-- then opens the port with that name for read and write after closing the previously open port.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::SetPort()
{
	mPort->close();
	mPort->setPortName(((QAction*)QObject::sender())->text());
	mPort->open(QSerialPort::ReadWrite);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: setFlag
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wan
--
-- PROGRAMMER:		Benny Wang
--
-- INTERFACE:		void setFlag(const uint32_t flag, const bool state)	
--						const uint32_t flag: The flag to be set.
--						const bool state: The state to set the flag too.
--
-- RETURNS:			void.
--
-- NOTES:
-- This function sets the target bit flag in the flag container to the state desired.
--
-- This function is thread safe.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::setFlag(const uint32_t flag, const bool state)
{
	mMutex.lock();
	if (state)
	{
		mFlags |= flag;
	}
	else
	{
		mFlags &= ~flag;
	}
	mMutex.unlock();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: isFlagSet
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang
--
-- PROGRAMMER:		Benny Wang
--
-- INTERFACE:		bool isFlagSet(const uint32_t flag)	
--						const uint32_t flag: The flag to check.
--
-- RETURNS:			True if the specified flag is set, otherwise false.
--
-- NOTES:
-- Check if the desired bit flag is set or not.
--
-- This function is thread safe.
----------------------------------------------------------------------------------------------------------------------*/
bool IOThread::isFlagSet(const uint32_t flag)
{
	bool result;
	mMutex.lock();
	result = mFlags & flag;
	mMutex.unlock();
	return result;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: startTimeout
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang
--
-- PROGRAMMER:		Benny Wang
--
-- INTERFACE:		void startTimeout(const int ms)	
--						const int ms: The length of the timeout in milliseconds.
--
-- RETURNS:			void.
--
-- NOTES:
-- Starts a timer.
--
-- Sets the timer and turns the TOR flag to true.
-- The timer is the current time plus the given ms plus a random ms.
-- The random ms is calculated with X * 100, where X is a random number between 0 and 9 inclusive.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::startTimeout(const int ms)
{
	mTimeout = QTime::currentTime().addMSecs(ms).addMSecs((qrand() % 10) * 100);
	setFlag(TOR, true);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: updateTimerout
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang
--
-- PROGRAMMER:		Benny Wang
--
-- INTERFACE:		void updateTimeout()	
--
-- RETURNS:			void.
--
-- NOTES:
-- Updates the TOR flag based on the current time.
-- If TOR is set, it will check if the end of the timeout has passed, if yes then the TOR flag is turned off. Otherwise
-- nothing is done.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::updateTimeout()
{
	if (isFlagSet(TOR))
	{
		if (QTime::currentTime() > mTimeout)
		{
			qDebug() << "turning timeout off";
			setFlag(TOR, false);
		}
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: SendFile
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang
--
-- INTERFACE:		void SendFile()	
--
-- RETURNS:			void.
--
-- NOTES:
-- This is Qt slot.
--
-- When the user wants to send a file, this function sets the RTS flag to true.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::SendFile()
{
	qDebug() << "turning rts on to send file";
	setFlag(RTS, true);
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: SetRVI()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Delan Elliot
--
-- INTERFACE:		void SetRVI()	
--
-- RETURNS:			void.
--
-- NOTES:
-- This is Qt slot.
--
-- Set the RVI flag to true.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::SetRVI()
{
	qDebug() << "setting rvi flag";
	setFlag(SEND_RVI, true);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendFrame
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang, Delan Elliot, Roger Zhang
--
-- INTERFACE:		void sendFrame()	
--
-- RETURNS:			void.
--
-- NOTES:
-- This function handles the transmission of a frame.
--
-- If there is data to send and the transmission session has not hit the cap a data frame is sent and the data frame
-- sent counter is incremented by 1 and the related flags are changed. If there is no data to send or if transmission
-- cap has been hit, the EOT frame is sent and a timer is set to force a back off session so the other side has a 
-- chance to transmit.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::sendFrame()
{
	if (mTxFrameCount < 10)
	{
		mRTXCount = 0;
		if (mFile->IsAtEndOfFile())
		{
			qDebug() << "end of file sending eot";
			setFlag(RTS, false);
			sendEOT();
		}
		else
		{
			writeToPort(makeFrame(mFile->GetNextBytes()));
			emit UpdateLabel("PacketReceived");
			setFlag(SENT_DATA, true);
			setFlag(RCV_ACK, false);
			mTxFrameCount++;
			qDebug() << "sendFrame starting timeout of 2s";
			startTimeout(TIMEOUT_LEN);
		}
	}
	else
	{
		qDebug() << "hit transmission cap, sending eot";
		sendEOT();
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: resendFrame
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang, Delan Elliot, Roger Zhang
--
-- INTERFACE:		void resendFrame()	
--
-- RETURNS:			void.
--
-- NOTES:
-- This function handles retransmission of the previous frame. 
--
-- If retransmissoin count has not been hit, retransmit the previous frame and increment the retransmission counter
-- and set the related flags. Otherwise teardown the session and go back to default state.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::resendFrame()
{
	if (mRTXCount < 3)
	{
		qDebug() << mRTXCount << "time resending frame" << mTxFrameCount;
		writeToPort(makeFrame(mFile->GetPreviousBytes()));
		setFlag(SENT_DATA, true);
		setFlag(RCV_ACK, false);
		mRTXCount++;
		qDebug() << "resendFrame starting timeout of 2s";
		startTimeout(TIMEOUT_LEN);
	}
	else
	{
		qDebug() << "hit max retransmit, backing off";
		resetFlags();
		qDebug() << "resendFrame starting timeout of 2s";
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendACK()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang, Delan Elliot
--
-- INTERFACE:		void jsendACK()	
--
-- RETURNS:			void.
--
-- NOTES:
-- Sends an ACK frame through the serial port, sets flags to represent that state, and starts a timer that waits
-- for a response.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::sendACK()
{
	emit writeToPort(ACK_FRAME);
	setFlag(SENT_ACK, true);
	setFlag(RCV_DATA, false);
	emit UpdateLabel("ACK");
	qDebug() << "sendACK starting timeout of 2s";
	startTimeout(TIMEOUT_LEN * 3);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendENQ()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang, Delan Elliot
--
-- INTERFACE:		void sendENQ()	
--
-- RETURNS:			void.
--
-- NOTES:
-- Sends an ENQ frame through the serial port, sets flags to represent that state, and starts a timer that waits
-- for a response.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::sendENQ()
{
	emit writeToPort(ENQ_FRAME);
	setFlag(SENT_ENQ, true);
	qDebug() << "sendENQ starting timeout of 2s x3";

	startTimeout(TIMEOUT_LEN);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendEOT()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang, Delan Elliot
--
-- INTERFACE:		void sendEOT()	
--
-- RETURNS:			void.
--
-- NOTES:
-- Sends an EOT frame through the serial port, sets flags to represent that state, and starts a timer that waits for 
-- a response.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::sendEOT()
{
	emit writeToPort(EOT_FRAME);
	mTxFrameCount = 0;
	setFlag(SENT_EOT, true);
	setFlag(FIN, true);
	setFlag(SENT_ENQ, false);
	qDebug() << "sendEOT starting timeout of 2s";
	startTimeout(TIMEOUT_LEN);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: sendRVI()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Delan Elliot	
--
-- INTERFACE:		void sendRVI()	
--
-- RETURNS:			void.
--
-- NOTES:
-- Sends an RVI frame through the serial port, sets flags to represent that state.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::sendRVI()
{
	emit writeToPort(RVI_FRAME);
	setFlag(SEND_RVI, false);
	resetFlagsNoTimeout();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: backoff()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Delan Elliot
--
-- INTERFACE:		void backoff()	
--
-- RETURNS:			void.
--
-- NOTES:
-- Puts the program in the backoff state as defined by the flags. In this state the program can only receive data and
-- can't send. This state is turned off when the timer that is started in this function expires.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::backoff()
{
	setFlag(FIN, true);
	setFlag(SENT_ENQ, false);
	qDebug() << "backoff starting timeout of 2s";
	startTimeout(TIMEOUT_LEN);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: GetDataFromPort()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang
--
-- INTERFACE:		void GetDataFromPort()	
--
-- RETURNS:			void.
--
-- NOTES:
-- This is a Qt slot.
--
-- When there is new data on the serial port it its read to a buffer and a function is called to check the buffer.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::GetDataFromPort()
{
	mBuffer += mPort->readAll();
	handleBuffer();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: handleBuffer()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang, Delan Elliot, Roger Zhang
--
-- INTERFACE:		void handleBuffer()	
--
-- RETURNS:			void.
--
-- NOTES:
-- When data is read to the buffer this function checks the data in the buffer.
--
-- If there is a control frame, the flags are sent to represent that control frame and the buffer is cleared.
-- If there is a data frame a function is called to handle the data frame.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::handleBuffer()
{
	if (mBuffer.contains(ENQ_FRAME))
	{
		qDebug() << "received enq";
		setFlag(RCV_ENQ, true);
		mBuffer.clear();
	}

	if (mBuffer.contains(ACK_FRAME))
	{
		qDebug() << "received ack";
		setFlag(RCV_ACK, true);
		setFlag(TOR, false);
		mBuffer.clear();

	}

	if (mBuffer.contains(EOT_FRAME))
	{
		qDebug() << "received eot";
		setFlag(RCV_EOT, true);
		mBuffer.clear();
	}

	if (mBuffer.contains(RVI_FRAME))
	{
		qDebug() << "received RVI";
		setFlag(RCV_RVI, true);
		mTxFrameCount = 0;
		mBuffer.clear();
	}

	if (mBuffer.contains(SYN_BYTE + STX_BYTE))
	{
		checkPotentialDataFrame();
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: checkPotentialDataFrame()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang, Delan Elliot, Roger Zhang
--
-- INTERFACE:		void checkPotentialDataFrame()	
--
-- RETURNS:			void.
--
-- NOTES:
-- This function is called when there is potentially a valid data frame in the buffer.
--
-- This function grabs the frame from the buffer and checks it. If the frame is a valid data frame, flags are sent to
-- represent that state and the data is extracted and the buffer gets cleared. Otherwise if the frame is not a valid
-- data frame, the flags are set to represent that state and a time is started.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::checkPotentialDataFrame()
{
	int dataFrameStart = mBuffer.indexOf(SYN_BYTE + STX_BYTE);
	QByteArray dataFrame = mBuffer.mid(dataFrameStart, DATA_FRAME_SIZE);

	if (isDataFrameValid(dataFrame))
	{
		qDebug() << "data frame valid";
		setFlag(RCV_DATA, true);
		setFlag(RCV_ERR, false);
		mFrameData = getDataFromFrame(dataFrame);
		mBuffer.clear();
	}
	else
	{
		qDebug() << "data frame invalid starting timeout of 6s";
		setFlag(RCV_DATA, true);
		setFlag(RCV_ERR, true);
		startTimeout(TIMEOUT_LEN * 3);
	}
	emit UpdateLabel(QString::number(byteError / (byteError + byteValid) * 100));
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: run()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Delan Elliot, Roger Zhang
--
-- INTERFACE:		void run()	
--
-- RETURNS:			void.
--
-- NOTES:
-- This funciton is the main entry point for Qt threads.
-- 
-- When the program first enters the thread all flags are reset.
-- 
-- While the program is running, this function will contuniously check the flags of the program and execute function
-- based on what state the program is in.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::run()
{
	resetFlags();

	while (mRunning)
	{
		updateTimeout();
		if (isFlagSet(SEND_RVI))
		{
			sendRVI();
		}

		if (isFlagSet(RCV_ENQ))
		{
			if (isFlagSet(FIN))
			{
				if (isFlagSet(SENT_ACK))
				{
					if (isFlagSet(RCV_EOT))
					{
						if (isFlagSet(RTS))
						{
							resetFlagsNoTimeout();
						}
						else
						{
							resetFlagsNoTimeout();
						}
					}
					else
					{
						if (isFlagSet(RCV_DATA))
						{
							if (isFlagSet(RCV_ERR))
							{
								setFlag(RCV_ERR, false);
								setFlag(RCV_DATA, false);
							}
							else
							{
								sendACK();
								emit DataReceieved(mFrameData);
							}
						}
						// RCV_data is false
						else
						{
							if (!isFlagSet(TOR))
							{
								resetFlagsNoTimeout();
							}
						}
					}
				}
				else
				{
					sendACK();
				}
			}
			else
			{
				setFlag(RCV_ENQ, false);
			}
		}
		// RCV_ENQ false
		else
		{
			if (isFlagSet(RCV_RVI))
			{
				resetFlags();
				continue;
			}

			if (isFlagSet(RTS))
			{
				if (isFlagSet(FIN))
				{
					if (isFlagSet(TOR))
					{
						continue;
					}
					else
					{
						setFlag(FIN, false);
					}
				}
				else
				{
					if (isFlagSet(SENT_ENQ))
					{
						if (isFlagSet(RCV_ACK))
						{
							sendFrame();
						}
						else
						{
							if (isFlagSet(SENT_DATA))
							{
								if (isFlagSet(TOR))
								{
									continue;
								}
								else
								{
									//retransmit
									resendFrame();
								}
							}
							else
							{
								if (isFlagSet(TOR))
								{
									continue;
								}
								else
								{
									//back off
									resetFlags();
								}
							}
						}
					}
					else
					{
						sendENQ();
					}
				}
			}
		}
		msleep(100);
	}
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: resetFlags()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang, Delan Elliot, Roger Zhang
--
-- INTERFACE:		void resetFlags()	
--
-- RETURNS:			void.
--
-- NOTES:
-- Resets all the flags except for RTS.
-- 
-- To reset flags, all flags are set to false except for RTS and FIN. RTS remains unchanged and FIN is set to true.
-- This funciton will also set a timeout.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::resetFlags()
{
	if (isFlagSet(RTS))
	{
		mMutex.lock();
		mFlags = 0;
		mFlags |= RTS | FIN;
		mMutex.unlock();
	}
	else
	{
		mMutex.lock();
		mFlags = 0;
		mFlags |= FIN;
		mMutex.unlock();
	}
	qDebug() << "resetFlags starting timeout of 2s";
	startTimeout(TIMEOUT_LEN);
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: resetFlags()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang, Delan Elliot, Roger Zhang
--
-- INTERFACE:		void resetFlags()	
--
-- RETURNS:			void.
--
-- NOTES:
-- Resets all the flags except for RTS.
-- 
-- To reset flags, all flags are set to false except for RTS and FIN. RTS remains unchanged and FIN is set to true.
-- This function will reset without setting a timeout.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::resetFlagsNoTimeout()
{
	if (isFlagSet(RTS))
	{
		mMutex.lock();
		mFlags = 0;
		mFlags |= RTS | FIN;
		mMutex.unlock();
	}
	else
	{
		mMutex.lock();
		mFlags = 0;
		mFlags |= FIN;
		mMutex.unlock();
	}
	qDebug() << "resetFlags no timeout";
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: writeToPort()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang
--
-- INTERFACE:		void writeToPort(const QByteArray& frame)
--						const QByteArray& frame: A reference to the QByteArray that will be written to the port.
--
-- RETURNS:			void.
--
-- NOTES:
-- Writes the given bites to the port.
----------------------------------------------------------------------------------------------------------------------*/
void IOThread::writeToPort(const QByteArray& frame)
{
	mPort->write(frame);
	mPort->flush();
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: makeFrame()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang
--
-- INTERFACE:		void makeFrame(const QByteArray& data)
--						const QByteArray& data: The data to wrap in a frame.
--
-- RETURNS:			void.
--
-- NOTES:
-- Wraps the given data in a frame.
--
-- A frame consists of a a header, data, and CRC-32. The header is a SYN byte followed by a STX byte. The data is a
-- string of regular ASCII of length 512. The CRC-32 is calculated on the 512 bytes of data only.
--
-- The details of the CRC-32 used are:
--		polynomial     = 0x04C11DB7
--		initial value  = 0xFFFFFFFF
--		final XOR      = 0xFFFFFFFF
--		reflect input  = true
--		reflect output = true
--		check value    = 0xCBF43926
----------------------------------------------------------------------------------------------------------------------*/
QByteArray IOThread::makeFrame(const QByteArray& data)
{
	QByteArray stuffedData = QByteArray(DATA_LENGTH - data.size(), 0x0);
	stuffedData.prepend(data);
	uint32_t crc = CRC::Calculate(stuffedData.data(), DATA_LENGTH, CRC::CRC_32());

	QByteArray frame = SYN_BYTE + STX_BYTE + stuffedData;
	frame = frame << crc;

	return frame;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: isDataFrameValid()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang
--
-- INTERFACE:		bool isDataFrameValid(const QByteArray& frame)
--						const QByteArray& frame: The incoming data frame.
--
-- RETURNS:			True if the incoming frame has no errors, otherwise false.	
--
-- NOTES:
-- This function checks that the frame is the right size and that the recalcuated CRC matches the sent CRC.
--
-- The details of the CRC-32 used are:
--		polynomial     = 0x04C11DB7
--		initial value  = 0xFFFFFFFF
--		final XOR      = 0xFFFFFFFF
--		reflect input  = true
--		reflect output = true
--		check value    = 0xCBF43926
----------------------------------------------------------------------------------------------------------------------*/
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

	double stuffingCount = frameData.count(char(0x0));
	recalculatedCrc == receivedCrc ? byteValid = byteValid + DATA_LENGTH - stuffingCount : byteError = byteError + DATA_LENGTH - stuffingCount;
	return recalculatedCrc == receivedCrc;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: getDataFromFrame()
--
-- DATE:			Dec 05, 2017
--
-- REVISIONS:		N/A 
--
-- DESIGNER:		Benny Wang, Delan Elliot, Roger Zhang, Juliana French
--
-- PROGRAMMER:		Benny Wang
--
-- INTERFACE:		QString getDataFromFrame(const QByteArray& frame)
--						const QByteArray& frame: A valid incoming data frame.
--
-- RETURNS:			The QString representation of the data in the frame.
--
-- NOTES:
-- This function extracts the data portion of a given valid data frame.
-----------------------------------------------------------------------------------------------------------------------*/
QString IOThread::getDataFromFrame(const QByteArray& frame)
{
	return frame.mid(DATA_HEADER_SIZE, DATA_LENGTH);
}

