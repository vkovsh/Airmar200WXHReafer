#pragma once

#include <QByteArray>

//#include <Qsl/Crc/Crc.h>

#include "SerialPort.h"
#include "RetCode.h"

namespace Airmar200WXH
{
    class IODev
    {
    public:
        IODev(const SerialPortSettings& serialPortSettings);
        ~IODev();
    public:
        RetCode open();
        void close();
        RetCode isOpen() const;
    public:
        RetCode recieve(QByteArray& buf, size_t timeout);
    private:
        IODev() = delete;
    private:
        SerialPort _dev;
    };
}
