#pragma once

#include <cstdint>

#include <QByteArray>

QByteArray &operator<<(QByteArray& l, uint8_t r);
QByteArray &operator<<(QByteArray& l, uint16_t r);
QByteArray &operator<<(QByteArray& l, uint32_t r);
