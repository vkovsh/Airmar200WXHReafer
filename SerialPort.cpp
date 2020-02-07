#include "SerialPort.h"

#include <qglobal.h>

#if defined(Q_OS_LINUX)
#include "SerialPort.lin.h"
#elif defined(Q_OS_WIN)
#include "SerialPort.win.h"
#else
#error __OS_UNKNOWN
#endif

using namespace Airmar200WXH;

SerialPort::SerialPort(const SerialPortSettings& settings)
    : _sys(new Sys(this))
    , _settings(settings)
{
}

SerialPort::~SerialPort()
{
    closeTty();
    delete _sys;
}

void SerialPort::setErr(Err err)
{
    _err = err;
}

void SerialPort::clearErr()
{
    _err = ERR_SUCCESS;
}

bool SerialPort::isOpen() const
{
    return _sys->isOpen();
}

bool SerialPort::openTty()
{
    if (isOpen())
    {
        setErr(SerialPort::ERR_OPEN);
        return false;
    }
    clearErr();
    if (_sys->openTty(_settings.port) == false)
    {
        setErr(SerialPort::ERR_OPEN);
        _sys->closeTty();
        return false;
    }
    return true;
}

void SerialPort::closeTty()
{
    if (!isOpen())
    {
        setErr(SerialPort::ERR_NOTOPEN);
        return;
    }
    _sys->closeTty();
}

int SerialPort::writeTty(const void* data, size_t size)
{
    return _sys->writeTty(data, size);
}

int SerialPort::writeTty(const char* data)
{
    return writeTty(data, strlen(data));
}

int SerialPort::readTty(void* data, const size_t size, const size_t timeoutMSec)
{
    return _sys->readTty(data, size, timeoutMSec);
}
