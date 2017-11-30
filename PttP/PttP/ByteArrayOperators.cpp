#include "ByteArrayOperators.h"

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: &operator<<()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Matteo Italia 
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: QByteArray &operatro<< (QByteArray &l, quint8 r)
--		QByteArray& l: The bytes that will have bytes appended to it.
--		quint8 r: Bytes to append to l in unsigned int 8 format.
--
-- RETURNS: A QByteArray that as r appended to l.
--
-- NOTES:
-- Appends bytes in the form of unsigned int 8 to a QByteArray.
-- 
-- This function was created by Matteo Italia on Stack Overflow.
-- https://stackoverflow.com/users/214671/matteo-italia
-------------------------------------------------------------------------------------------------*/
QByteArray &operator<<(QByteArray& l, quint8 r)
{
	l.append(r);
	return l;
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: &operator<<()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Matteo Italia 
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: QByteArray &operatro<< (QByteArray &l, quint8 r)
--		QByteArray& l: The bytes that will have bytes appended to it.
--		quint8 r: Bytes to append to l in unsigned int 8 format.
--
-- RETURNS: A QByteArray that as r appended to l.
--
-- NOTES:
-- Appends bytes in the form of unsigned int 8 to a QByteArray.
-- 
-- This function was created by Matteo Italia on Stack Overflow.
-- https://stackoverflow.com/users/214671/matteo-italia
-------------------------------------------------------------------------------------------------*/
QByteArray &operator<<(QByteArray& l, quint16 r)
{
	return l << quint8(r >> 8) << quint8(r);
}

/*-------------------------------------------------------------------------------------------------
-- FUNCTION: &operator<<()
--
-- DATE: November 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Matteo Italia 
--
-- PROGRAMMER: Benny Wang 
--
-- INTERFACE: QByteArray &operatro<< (QByteArray &l, quint8 r)
--		QByteArray& l: The bytes that will have bytes appended to it.
--		quint8 r: Bytes to append to l in unsigned int 8 format.
--
-- RETURNS: A QByteArray that as r appended to l.
--
-- NOTES:
-- Appends bytes in the form of unsigned int 8 to a QByteArray.
-- 
-- This function was created by Matteo Italia on Stack Overflow.
-- https://stackoverflow.com/users/214671/matteo-italia
-------------------------------------------------------------------------------------------------*/
QByteArray &operator<<(QByteArray& l, quint32 r)
{
	return l << quint16(r >> 16) << quint16(r);
}