#pragma once

#include <iomanip>
#include <cstdint>

#include <QAction>
#include <QByteArray>
#include <QObject>
#include <QSerialPort>
#include <QString>
#include <QThread>

#include <QMessageBox>

#include "CRC.h"

#include "ByteArrayOperators.h"
#include "ControlCharacters.h"
#include "FileManip.h"

#define DATA_FRAME_SIZE	518
#define DATA_HEADER_SIZE 2
#define DATA_LENGTH	512

#define RTS		 0x01
#define CTS		 0x02
#define RCV_ACK  0x04
#define RCV_ENQ  0x08
#define RCV_EOT  0x10
#define RCV_DATA 0x20
#define WAIT_RCV 0x40

#define TIMEOUT 2000

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
	inline QSerialPort* GetPort()
	{
		return mPort;
	}

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
	inline FileManip* GetFileManip()
	{
		return mFile;
	}

	void SendACK();
	void SendENQ();
	void SendEOT();

protected:
	void run();

private:
	bool mRunning;

	uint32_t mFlag;

	FileManip* mFile;
	QSerialPort* mPort;

	QByteArray mBuffer;
	int mTxFrameCount;

	void sendFrame();
	QByteArray makeFrame(const QByteArray& data);

	bool isDataFrameValid(const QByteArray& frame);
	QString getDataFromFrame(const QByteArray& frame);
	
	void handleBuffer();
	void handleENQ();
	void handleEOT();
	void handleIncomingDataFrame();

public slots:
	void SendFile();
	void GetDataFromPort();
	void SetPort();
	void writeToPort(const QByteArray& frame);

signals:
	void DataReceieved(const QString data);
	void writeToPortSignal(const QByteArray& frame);
};
