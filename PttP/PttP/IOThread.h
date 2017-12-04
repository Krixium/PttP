#pragma once

#include <iomanip>
#include <cstdint>

#include <QAction>
#include <QByteArray>
#include <QObject>
#include <QSerialPort>
#include <QString>
#include <QThread>

#include "CRC.h"

#include "ByteArrayOperators.h"
#include "ControlCharacters.h"
#include "FileManip.h"

#define DATA_FRAME_SIZE	518
#define DATA_HEADER_SIZE 2
#define DATA_LENGTH	512

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

	bool mRequestingToSend;
	bool mClearToSend;

	bool mACKReceived;
	bool mENQReceived;
	bool mEOTReceived;
	bool mDataReceived;

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
