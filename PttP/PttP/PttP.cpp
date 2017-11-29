#include "PttP.h"

using namespace std;

PttP::PttP(QWidget *parent)
	: QMainWindow(parent)
	, mIOThread(new IOThread(this))
	, mFile(new FileManip(this))
{
	ui.setupUi(this);

	connect(ui.pushButtonSelect, &QPushButton::pressed, mFile, &FileManip::SelectFile);
	connect(mFile, &FileManip::fileChanged, this, &PttP::SetFileName);

	connect(ui.pushButtonStart, &QPushButton::pressed, mIOThread, &IOThread::SendENQ);

	connect(mIOThread, &IOThread::LineReadyToSend, this, &PttP::SendBytesOverPort);

	mIOThread->start();
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
	mIOThread->Send(mFile->GetNextBytes());
}