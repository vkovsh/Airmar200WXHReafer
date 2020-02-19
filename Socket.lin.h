#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/socket.h>
#include <arpa/inet.h>

namespace Airmar200WXH
{
    class Socket::Sys
    {
    public:
        Sys(Socket* parent)
            : _parent(parent) {}

        bool isOpen() const { return (_handle != -1); }

        bool openTty(std::string port)
        {
            {
                _parent->servAddr.sin_family = _parent->_settings.sin_family;
                _parent->servAddr.sin_port = htons(_parent->_settings.sin_port);
                if (inet_pton(_parent->servAddr.sin_family,
                          _parent->_settings.sin_addr.to_char(),
                          &(_parent->servAddr.sin_addr)) <= 0)
                {
                    return false;
                }

            }

            _handle = socket(AF_INET, SOCK_STREAM, 0);
            if (_handle == -1)
            {
                _parent->setErr(getSystemError());
                return false;
            }

            if (connect(_handle, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                printf("\nConnection Failed \n");
                return -1;
            }
            return true;
        }

        void closeTty()
        {
            if (isOpen() == true)
            {
                setTermios(&_restoredTio);
                if (false == close(_handle))
                    _parent->setErr(getSystemError());
                _handle = -1;
            }
        }

        int readTty(void* data, const size_t size, const size_t timeoutMsec)
        {
            if (isOpen() == false)
            {
                _parent->setErr(getSystemError());
                return false;
            }


        }

        int writeTty(const void* data, const size_t size)
        {
        }

    private:
        bool initTty()
        {
            if (isOpen() == false)
                return false;
            return true;
        }

        bool getTermios(struct termios *comOptions)
        {
            memset(comOptions, 0, sizeof(struct termios));
            if (tcgetattr(_handle, comOptions) != 0)
            {
                _parent->setErr(getSystemError());
                return false;
            }
            return true;
        }

        bool setTermios(struct termios *comOptions)
        {
            tcflush (_handle, TCIFLUSH);
            if (0 != tcsetattr(_handle, TCSANOW, comOptions))
            {
                _parent->setErr(getSystemError());
                return false;
            }
            return true;
        }
    private:
        Socket::Err getSystemError(int systemErrorCode = -1) const
        {
            if (systemErrorCode == -1)
                systemErrorCode = errno;

            SerialPort::Err err;

            switch (systemErrorCode)
            {
                case 0:
                    err = Socket::ERR_SUCCESS;
                    break;
                case EPERM:
                    err = Socket::ERR_PERMISSION;
                    break;
                case ENOENT:
                    err = Socket::ERR_DEVICENOTFOUND;
                    break;
                case EBADE:
                    err = Socket::ERR_DESCRIPTOR;
                    break;
                case EIO:
                    err = Socket::ERR_IO;
                    break;
                default:
                    err = Socket::ERR_UNKNOWN;
                    break;
            }
            return err;
        }

    private:
        Socket* _parent = nullptr;
        int _handle = -1;
        struct termios _restoredTio;
    };
}
