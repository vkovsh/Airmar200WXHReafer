#include "SerialPort.h"

#include <qglobal.h>

#if defined(Q_OS_LINUX)
#include "Socket.lin.h"
#elif defined(Q_OS_WIN)
#include "Socket.win.h"
#else
#error __OS_UNKNOWN
#endif

using namespace Airmar200WXH;

Socket::Socket(const SocketClientSettings& settings)
    : _sys(new Sys(this))
    , _settings(settings)
{
}

Socket::~Socket()
{
    closeTty();
    delete _sys;
}

void Socket::setErr(Err err)
{
    _err = err;
}

void Socket::clearErr()
{
    _err = ERR_SUCCESS;
}

bool Socket::isOpen() const
{
    return _sys->isOpen();
}

bool Socket::openTty()
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

void Socket::closeTty()
{
    if (!isOpen())
    {
        setErr(Socket::ERR_NOTOPEN);
        return;
    }
    _sys->closeTty();
}

int Socket::writeTty(const void* data, size_t size)
{
    return _sys->writeTty(data, size);
}

int Socket::writeTty(const char* data)
{
    return writeTty(data, strlen(data));
}

int Socket::readTty(void* data, const size_t size, const size_t timeoutMSec)
{
    return _sys->readTty(data, size, timeoutMSec);
}
