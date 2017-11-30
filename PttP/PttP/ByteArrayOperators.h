#pragma once
#include <QByteArray>

QByteArray &operator<<(QByteArray& l, quint8 r);
QByteArray &operator<<(QByteArray& l, quint16 r);
QByteArray &operator<<(QByteArray& l, quint32 r);
