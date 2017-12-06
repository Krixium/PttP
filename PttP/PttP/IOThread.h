#pragma once

#include <cstdint>
#include <iomanip>

#include <QAction>
#include <QByteArray>
#include <QMutex>
#include <QObject>
#include <QSerialPort>
#include <QString>
#include <QThread>
#include <QTime>

#include "CRC.h"

#include "ByteArrayOperators.h"
#include "ControlCharacters.h"
#include "FileManip.h"

#define DATA_FRAME_SIZE 518
#define DATA_HEADER_SIZE 2
#define DATA_LENGTH	512

#define RTS			0x0001
#define FIN			0x0002
#define RCV_ENQ		0x0004
#define RCV_ACK		0x0008
#define RCV_DATA	0x0010
#define RCV_EOT		0x0020
#define RCV_ERR		0x0040
#define SENT_ENQ	0x0080
#define SENT_ACK	0x0100
#define SENT_DATA	0x0200
#define SENT_EOT	0x0400
#define TOR			0x0800
#define RCV_RVI     0x1000
#define SEND_RVI    0x2000

#define TIMEOUT_LEN 2000

using namespace std;

class IOThread : public QThread
{
	Q_OBJECT

public:
	const static QByteArray SYN_BYTE;
	const static QByteArray STX_BYTE;

	const static QByteArray ACK_FRAME;
	const static QByteArray ENQ_FRAME;
	const static QByteArray EOT_FRAME;
	const static QByteArray RVI_FRAME;

	IOThread(QObject *parent);
	~IOThread();

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
	inline QSerialPort* GetPort() const { return mPort; }

	/*-------------------------------------------------------------------------------------------------
	-- FUNCTION: GetFileManip()
	--
	-- DATE: November 29, 2017
	--
	-- REVISIONS: N/A
	--
	-- DESIGNER: Benny Wang
	--
	-- PROGRAMMER: Benny Wang
	--
	-- INTERFACE: FileManip* GetFileManip (void)
	--
	-- RETURNS: A pointer to the File Manipulator.
	--
	-- NOTES:
	-- Getter function for the pointer to the programs File manipulator.
	-------------------------------------------------------------------------------------------------*/
	inline FileManip* GetFileManip() const { return mFile; }

protected:
	void run();

private:
	bool mRunning;
	uint32_t mFlags;

	FileManip* mFile;
	QSerialPort* mPort;
	QMutex mMutex;

	QByteArray mBuffer;
	QString mFrameData;
	int mTxFrameCount;
	int mRTXCount;
	QTime mTimeout;


	void setFlag(const uint32_t flag, const bool state);
	bool isFlagSet(const uint32_t flag);

	void startTimeout(const int ms);
	void updateTimeout();

	void sendACK();
	void sendENQ();
	void sendEOT();
	void sendRVI();
	void sendFrame();
	void resendFrame();
	void resetFlags();
	void resetFlagsNoTimeout();
	void backoff();
	QByteArray makeFrame(const QByteArray& data);

	void handleBuffer();
	void checkPotentialDataFrame();

	bool isDataFrameValid(const QByteArray& frame);
	QString getDataFromFrame(const QByteArray& frame);

public slots:
	void SendFile();
	void SetRVI();
	void GetDataFromPort();
	void SetPort();
	void writeToPort(const QByteArray& frame);

signals:
	void DataReceieved(const QString data);
	void writeToPortSignal(const QByteArray& frame);
};
