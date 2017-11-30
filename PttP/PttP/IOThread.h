#pragma once

#include <QAction>
#include <QByteArray>
#include <QObject>
#include <QSerialPort>
#include <QThread>

#include "ByteArrayOperators.h"

using namespace std;

class IOThread : public QThread
{
	Q_OBJECT

public:
	const static QByteArray SYN;
	const static QByteArray STX;

	const static QByteArray ACK_FRAME;
	const static QByteArray ENQ_FRAME;
	const static QByteArray EOT_FRAME;

	IOThread(QObject *parent);
	~IOThread();

	QSerialPort* GetPort();

	void Send(const QByteArray& data);

protected:
	void run();

private:
	bool mRunning;
	QSerialPort* mPort;

	QByteArray makeFrame(const QByteArray& data);

public slots:
	void SendACK();
	void SendENQ();
	void GetDataFromPort();
	void SetPort();

signals:
	void LineReadyToSend();
	void DataFrameRecieved(QByteArray frame);
};
