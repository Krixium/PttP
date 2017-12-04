#include "FileManip.h"

FileManip::FileManip(QObject* parent)
	: mFile("")
	, mInStream(make_unique<ifstream>())
{
}

FileManip::~FileManip()
{
	mInStream->close();
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: SelectFile()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: void selectFile (void)
--
-- RETURNS: void.
--
-- NOTES:
--
-- A Qt Slot.
--
-- Opens a QFileDialog to retrieve the filename of the file that the user wants to transfer and
-- saves it to an instance variable of this class. After that the new file name is emiited so that
-- it can be displayed.
-------------------------------------------------------------------------------------------------*/
void FileManip::SelectFile()
{
	mFile = QFileDialog::getOpenFileName(
		this,							// Parent object
		tr("Choose File to Open"),		// Title
		"./",							// Default directory
		tr("Text File ( *.txt)")		// File types
	).toStdString();

	mInStream->open(mFile, fstream::in | fstream::binary);

	emit fileChanged(mFile);
}


/*-------------------------------------------------------------------------------------------------
-- FUNCTION: GetNextBytes()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang, Delan Elliot
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: QByteArray GetNextBytes (const int)
--		const int numOfBytes: The number of bytes you want to read from the file.
--
-- RETURNS: The number of bytes the user requested from the current file pointer.
--
-- NOTES:
--
-- Read 512 bytes from the current file position.
-- 
-- The bytes read are also saved to a buffer and can be retrieved again using GetPreviousBytes().
-------------------------------------------------------------------------------------------------*/
QByteArray FileManip::GetNextBytes()
{
	mInStream->get(mBuffer, 512, EOF);
	return QByteArray(mBuffer);
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: GetPreviousBytes()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Benny Wang
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: QByteArray GetPreviousBytes ()
--
-- RETURNS: The last array of bytes requested from the FileManip
--
-- NOTES:
--
-- Gives the users the same bytes that the previous GetNextByte call returned. This is used for
-- retransmission.
-------------------------------------------------------------------------------------------------*/
QByteArray FileManip::GetPreviousBytes()
{
	return QByteArray(mBuffer);
}

bool FileManip::IsAtEndOfFile()
{
	return mInStream->eof();
}