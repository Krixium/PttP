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
	, mFlags(0)
	, mPort(new QSerialPort(this))
	, mFile(new FileManip(this))
	, mTxFrameCount(0)
	, mRTXCount(0)
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

void IOThread::SetPort()
{
	mPort->close();
	mPort->setPortName(((QAction*)QObject::sender())->text());
	mPort->open(QSerialPort::ReadWrite);
}

void IOThread::setFlag(uint32_t flag, bool state)
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

bool IOThread::isFlagSet(const uint32_t flag)
{
	bool result;
	mMutex.lock();
	result = mFlags & flag;
	mMutex.unlock();
	return result;
}

void IOThread::startTimeout(const int ms)
{
	mTimeout = QTime::currentTime().addMSecs(ms).addMSecs((qrand() % 10) * 100);
	setFlag(TOR, true);
}

void IOThread::updateTimeout()
{
	if (isFlagSet(TOR))
	{
		if (QTime::currentTime() > mTimeout)
		{
			setFlag(TOR, false);
		}
	}
}

void IOThread::SendFile()
{
	setFlag(RTS, true);
}

void IOThread::sendFrame()
{
	writeToPort(makeFrame(mFile->GetNextBytes()));
	setFlag(SENT_DATA, true);
	setFlag(RCV_ACK, false);
	startTimeout(TIMEOUT_LEN);
}

void IOThread::resendFrame()
{
	if (mRTXCount < 3)
	{
		writeToPort(makeFrame(mFile->GetPreviousBytes()));
		setFlag(SENT_DATA, true);
		setFlag(RCV_ACK, false);
		mRTXCount++;
		startTimeout(TIMEOUT_LEN);
	}
	else
	{
		setFlag(RCV_ENQ, false);
		setFlag(RTS, true);
		setFlag(FIN, true);
		startTimeout(TIMEOUT_LEN);
	}
}

void IOThread::sendACK()
{
	qDebug() << "sending ack";
	emit writeToPort(ACK_FRAME);
	setFlag(SENT_ACK, true);
	startTimeout(TIMEOUT_LEN * 3);
}

void IOThread::sendENQ()
{
	qDebug() << "sending enq";
	emit writeToPort(ENQ_FRAME);
	setFlag(SENT_ENQ, true);
	startTimeout(TIMEOUT_LEN * 3);
}

void IOThread::sendEOT()
{
	qDebug() << "sending eot";
	emit writeToPort(EOT_FRAME);
	setFlag(SENT_EOT, true);
	setFlag(FIN, true);
	startTimeout(TIMEOUT_LEN);
}


void IOThread::GetDataFromPort()
{
	mBuffer += mPort->readAll();
	handleBuffer();
}

void IOThread::handleBuffer()
{
	if (mBuffer.contains(ENQ_FRAME))
	{
		setFlag(RCV_ENQ, true);
		mBuffer.clear();
	}

	if (mBuffer.contains(ACK_FRAME))
	{
		setFlag(RCV_ACK, true);
		mBuffer.clear();
	}

	if (mBuffer.contains(EOT_FRAME))
	{
		setFlag(RCV_EOT, true);
		mBuffer.clear();
	}

	if (mBuffer.contains(SYN_BYTE + STX_BYTE))
	{
		checkPotentialDataFrame();
	}
}

void IOThread::checkPotentialDataFrame()
{
	int dataFrameStart = mBuffer.indexOf(SYN_BYTE + STX_BYTE);
	QByteArray dataFrame = mBuffer.mid(dataFrameStart, DATA_FRAME_SIZE);

	qDebug() << "incoming dataframe =" << dataFrame;
	qDebug() << "incoming frame size =" << dataFrame.size();

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
		qDebug() << "data frame invalid";
		setFlag(RCV_DATA, true);
		setFlag(RCV_ERR, true);
		startTimeout(TIMEOUT_LEN * 3);
	}
}

////////////////////////////////////////////////////////////////////////////////
///								  Main Loop  								 ///
////////////////////////////////////////////////////////////////////////////////

void IOThread::run()
{
	while (mRunning)
	{
		updateTimeout();

		if (isFlagSet(RCV_ENQ))
		{
			if (isFlagSet(FIN))
			{
				if (isFlagSet(SENT_ACK))
				{
					if (isFlagSet(RCV_DATA))
					{
						checkPotentialDataFrame();
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
							resetFlags();
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
				setFlag(!RCV_ENQ, false);
			}
		}
		// RCV_ENQ false
		else
		{
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
									setFlag(FIN, true);
									startTimeout(TIMEOUT_LEN);
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
	}
}

void IOThread::resetFlags()
{
	setFlag(RCV_ENQ, false);
	setFlag(RTS, true);
	setFlag(FIN, true);
	startTimeout(TIMEOUT_LEN);
}

////////////////////////////////////////////////////////////////////////////////
///							Serial Port Functions							 ///
////////////////////////////////////////////////////////////////////////////////

void IOThread::writeToPort(const QByteArray& frame)
{
	mPort->write(frame);
	mPort->flush();
}

QByteArray IOThread::makeFrame(const QByteArray& data)
{
	QByteArray stuffedData = QByteArray(DATA_LENGTH - data.size(), 0x0);
	stuffedData.prepend(data);
	uint32_t crc = CRC::Calculate(stuffedData.data(), DATA_LENGTH, CRC::CRC_32());

	QByteArray frame = SYN_BYTE + STX_BYTE + stuffedData;
	frame = frame << crc;

	return frame;
}

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

QString IOThread::getDataFromFrame(const QByteArray& frame)
{
	return frame.mid(DATA_HEADER_SIZE, DATA_LENGTH);
}

