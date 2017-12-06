#pragma once

#include <string>

#include <QtWidgets/QMainWindow>
#include <QScrollBar>
#include <QSerialPortInfo>
#include <QPlainTextEdit>

#include "FileManip.h"
#include "IOThread.h"
#include "ui_PttP.h"

using namespace std;

class PttP : public QMainWindow
{
	Q_OBJECT

public:
	PttP(QWidget *parent = Q_NULLPTR);

private:
	Ui::PttPClass ui;

	IOThread* mIOThread;

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
	-- INTERFACE: void populatePortMenu (void)
	--
	-- RETURNS: void
	--
	-- NOTES:
	-- Populates the port menu in the menu bar with a list of all available serial ports on the system.
	-------------------------------------------------------------------------------------------------*/
	void populatePortMenu();

	unsigned int numACK = 0;
	unsigned int numPackets = 0;

	public slots:
	void SetFileName(const string newFileName);

	void DisplayDataFromPort(const QString data);

	void UpdateLabel(const QString str);
};
