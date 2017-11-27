#include "PttP.h"

PttP::PttP(QWidget *parent)
	: QMainWindow(parent)
	, mSelectedFileName()
{
	ui.setupUi(this);
	connect(ui.pushButtonSelect, &QPushButton::pressed, this, &PttP::selectFile);
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: selectFile()
--
-- DATE: November 26, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: 
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: void selectFile (void)
--
-- RETURNS: void.
--
-- NOTES:
-- A Qt slot.
--
-- Opens a QFileDialog to retrieve the filename of the file that the user wants to transfer and
-- saves it to an instance variable of this class.
-------------------------------------------------------------------------------------------------*/
void PttP::selectFile()
{
	mSelectedFileName = QFileDialog::getOpenFileName(
		this,							// Parent object
		tr("Choose File to Open"),		// Title
		"./",							// Default directory
		tr("Text File (*.txt)")			// File types
	);
	ui.labelSelectedFile->setText(mSelectedFileName);
}