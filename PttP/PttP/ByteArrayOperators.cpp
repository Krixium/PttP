/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: ByteArrayOperators.cpp - A group of operator overloads for appending uints.
--
-- PROGRAM: PttP
--
-- FUNCTIONS:
-- QByteArray &operator<<(QByteArray& l, unint8_t r)
-- QByteArray &operator<<(QByteArray& l, unint16_t r)
-- QByteArray &operator<<(QByteArray& l, unint32_t r)
--
-- DATE: Nov 29, 2017
--
-- REVISIONS: N/A
--
-- DESIGNER: Matteo Italia
--
-- PROGRAMMER: Benny Wang
--
-- NOTES:
-- The function in this file allow for a block of binary to be converted to bytes and appended to a QByteArray.
--
-- Credit for creating these functions goes to Matteo Italia on Stack Overflow.
-- https://stackoverflow.com/users/214671/matteo-italia
----------------------------------------------------------------------------------------------------------------------*/
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
QByteArray &operator<<(QByteArray& l, uint8_t r)
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
QByteArray &operator<<(QByteArray& l, uint16_t r)
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
QByteArray &operator<<(QByteArray& l, uint32_t r)
{
	return l << quint16(r >> 16) << quint16(r);
}