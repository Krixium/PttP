#pragma once

#include <QByteArray>
#include <QSerialPort>
#include <QThread>

using namespace std;

class IOThread : public QThread
{
	Q_OBJECT

public:
	const static QByteArray ACK;
	const static QByteArray ENQ;

	IOThread(QObject *parent);
	~IOThread();

	QSerialPort* GetPort();
	QString GetDataFromPort();

	void Send(const QByteArray data);

protected:
	void run();

private:
	bool mRunning;
	QSerialPort* mPort;

public slots:
	void SendACK();
	void SendENQ();

signals:
	void LineReadyToSend();
};
