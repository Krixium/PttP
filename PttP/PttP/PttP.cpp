#include "PttP.h"

#include <QDebug>

using namespace std;

PttP::PttP(QWidget *parent)
	: QMainWindow(parent)
	, mIOThread(new IOThread(this))
	, mFile(new FileManip(this))
{
	ui.setupUi(this);

	populatePortMenu();

	// Menu option exit
	connect(ui.actionExit, &QAction::triggered, this, &QWidget::close);

	// Selecting a file
	connect(ui.pushButtonSelect, &QPushButton::pressed, mFile, &FileManip::SelectFile);
	connect(mFile, &FileManip::fileChanged, this, &PttP::SetFileName);

	// Start button to send ENQ
	connect(ui.pushButtonStart, &QPushButton::pressed, mIOThread, &IOThread::SendENQ);

	// Send data when line is ready
	connect(mIOThread, &IOThread::LineReadyToSend, this, &PttP::SendBytesOverPort);

	mIOThread->start();
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: populatePortMenu()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: void populatePortMenu ()
--
-- RETURNS: void.
--
-- NOTES:
-- Populates the port menu in the menu bar with a list of all available serial ports on the system.
-------------------------------------------------------------------------------------------------*/
void PttP::populatePortMenu()
{
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
	if (ports.size() == 0)
	{
		ui.menuPorts->setEnabled(false);
		return;
	}

	QAction* action;

	for (int i = 0; i < ports.size(); i++)
	{
		action = new QAction(this);
		action->setObjectName(ports[i].portName());
		action->setText(ports[i].portName());

		ui.menuPorts->addAction(action);

		connect(action, &QAction::triggered, mIOThread, &IOThread::SetPort);
	}
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: SetFileName()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: void SetFileName (const string)
--		const string newFileName: The full path and name of the new file.
--
-- RETURNS: void.
--
-- NOTES:
-- Populates the file name label with the full path and name of the new file.
-------------------------------------------------------------------------------------------------*/
void PttP::SetFileName(const string newFileName)
{
	ui.labelSelectedFile->setText(QString(newFileName.c_str()));
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: SendBytesOverPort()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: void SendBytesOverPort ()
--
-- RETURNS: void.
--
-- NOTES:
-- Queries the FileManip object for the enxt 512 bytes in the selected file and sends it to the
-- serial port to be transmitted.
-------------------------------------------------------------------------------------------------*/
void PttP::SendBytesOverPort()
{
	qDebug() << "Sending bytes to the port";
	mIOThread->Send(mFile->GetNextBytes());
}