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
	, byteError(0)
	, byteValid(0)
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
			qDebug() << "turning timeout off";
			setFlag(TOR, false);
		}
	}
}

void IOThread::SendFile()
{
	qDebug() << "turning rts on to send file";
	setFlag(RTS, true);
}

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

void IOThread::sendACK()
{
	emit writeToPort(ACK_FRAME);
	setFlag(SENT_ACK, true);
	setFlag(RCV_DATA, false);
	emit UpdateLabel("ACK");
	qDebug() << "sendACK starting timeout of 2s";
	startTimeout(TIMEOUT_LEN * 3);
}

void IOThread::sendENQ()
{
	emit writeToPort(ENQ_FRAME);
	setFlag(SENT_ENQ, true);
	qDebug() << "sendENQ starting timeout of 2s x3";
	qDebug() << "RTS" << isFlagSet(RTS);
	qDebug() << "FIN" << isFlagSet(FIN);
	qDebug() << "RCV_ENQ" << isFlagSet(RCV_ENQ);
	qDebug() << "RCV_ACK" << isFlagSet(RCV_ACK);
	qDebug() << "RCV_DATA" << isFlagSet(RCV_DATA);
	qDebug() << "RCV_EOT" << isFlagSet(RCV_EOT);
	qDebug() << "RCV_ERR" << isFlagSet(RCV_ERR);
	qDebug() << "SENT_ENQ" << isFlagSet(SENT_ENQ);
	qDebug() << "SENT_ACK" << isFlagSet(SENT_ACK);
	qDebug() << "SENT_DATA" << isFlagSet(SENT_DATA);
	qDebug() << "SENT_EOT" << isFlagSet(SENT_EOT);
	qDebug() << "TOR" << isFlagSet(TOR);

	startTimeout(TIMEOUT_LEN);
}

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

void IOThread::backoff()
{
	setFlag(FIN, true);
	setFlag(SENT_ENQ, false);
	qDebug() << "backoff starting timeout of 2s";
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

	if (mBuffer.contains(SYN_BYTE + STX_BYTE))
	{
		checkPotentialDataFrame();
	}
}

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
	emit UpdateLabel(QString::number(byteError / byteValid));
}

////////////////////////////////////////////////////////////////////////////////
///								  Main Loop  								 ///
////////////////////////////////////////////////////////////////////////////////

void IOThread::run()
{
	resetFlags();

	while (mRunning)
	{
		updateTimeout();

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
		msleep(50);
	}
}

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

	int stuffingCount = frameData.count(char(0x0));
	recalculatedCrc == receivedCrc ? byteValid = byteValid + DATA_LENGTH - stuffingCount : byteError = byteError + DATA_LENGTH - stuffingCount;
	return recalculatedCrc == receivedCrc;
}

QString IOThread::getDataFromFrame(const QByteArray& frame)
{
	return frame.mid(DATA_HEADER_SIZE, DATA_LENGTH);
}

