#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <string.h>

//#include <QDebug>
//#include "../DbgTimeElapsed.h"

namespace Airmar200WXH
{
    class SerialPort::Sys
    {
    public:
        Sys(SerialPort* parent)
            : _parent(parent) {}

        bool isOpen() const { return (_handle != -1); }

        bool openTty(std::string port)
        {
            port = "/dev/" + port;
            _handle = open(port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
            if (_handle == -1)
            {
                _parent->setErr(getSystemError());
                return false;
            }
            fcntl(_handle, F_SETFL, (O_APPEND | O_NONBLOCK) & ~FNDELAY);

            if (false == initTty())
            {
                return false;
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

            fd_set rfds;
			FD_ZERO(&rfds);
			FD_SET(_handle, &rfds);

			struct timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = timeoutMsec * 1000;

            int retval = select(_handle + 1, &rfds, NULL, NULL, &timeout);
            if (retval != 0 && retval != -1)
            {
                int bytesCount = read(_handle, data, size);
                if (bytesCount > 0)
                {
                    return bytesCount;
                }
                _parent->setErr(getSystemError());
                return bytesCount;
            }
            _parent->setErr(getSystemError());
            return retval;
        }

        int writeTty(const void* data, const size_t size)
        {
            if (isOpen() == false)
            {
                _parent->setErr(getSystemError());
                return false;
            }

            int numberOfBytesWritten = write(_handle, data, size);
            if (numberOfBytesWritten > 0)
                return numberOfBytesWritten;
            _parent->setErr(getSystemError());
            return -1;
        }

    private:
        bool initTty()
        {
            if (isOpen() == false)
                return false;

            //Get the existing Comm Port Attributes in cwrget
            constexpr speed_t BAUNDRATES[SerialPortSettings::BR_TOTAL] = {B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200};
			if(uint32_t(_parent->_settings.baudRate) >= SerialPortSettings::BR_TOTAL)
            {
				return false;
            }
            const speed_t BAUDRATE = BAUNDRATES[_parent->_settings.baudRate];

            struct termios tio;
            if (getTermios(&tio) == false)
            {
                return false;
            }
            _restoredTio = tio;

            //Set the Tx and Rx Baud Rate to BAUDRATE
            cfsetospeed(&tio, BAUDRATE);
            cfsetispeed(&tio, BAUDRATE);

            //Enable the Receiver and  Set local Mode
            tio.c_iflag = IGNBRK;               /* Ignore Break Condition & no processing under input options*/
            tio.c_lflag = 0;                    /* Select the RAW Input Mode through Local options*/
            tio.c_oflag = 0;                    /* Select the RAW Output Mode through Local options*/
            tio.c_cflag |= (CLOCAL | CREAD);	/* Select the Local Mode & Enable Receiver through Control options*/

            //Make RAW Mode more explicit by turning Canonical Mode off, Echo off, Echo Erase off and Signals off*/
            tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

            //Disable Software Flow Control
            tio.c_iflag &= ~(IXON | IXOFF | IXANY);

            //Set Data format to 8N1
            tio.c_cflag &= ~CSIZE;  /* Mask the Character Size Bits through Control options*/

            /* This specifies count of bits per byte*/
            {
                switch (_parent->_settings.dataBits)
                {
                    case SerialPortSettings::DB_5:
                        tio.c_cflag |= CS5;
                        break;
                    case SerialPortSettings::DB_6:
                        tio.c_cflag |= CS6;
                        break;
                    case SerialPortSettings::DB_7:
                        tio.c_cflag |= CS7;
                        break;
                    case SerialPortSettings::DB_8:
                        tio.c_cflag |= CS8;
                        break;
                    default:
                        tio.c_cflag |= CS8;
                        break;
                }
            }

            {
                switch (_parent->_settings.stopBits)
                {
                    case SerialPortSettings::SB_ONE:
                        tio.c_cflag &= ~CSTOPB;
                        break;
                    case SerialPortSettings::SB_TWO:
                        tio.c_cflag &= CSTOPB;
                        break;
                    default:
                        tio.c_cflag &= ~CSTOPB;
                        break;
                }
            }

            {
                switch (_parent->_settings.parity)
                {
                    case SerialPortSettings::Parity::PY_NO:
                        tio.c_cflag &= ~PARENB;                              /* Select Parity Disable through Control options*/
                        break;
                    case SerialPortSettings::PY_EVEN:
                        tio.c_iflag |= (INPCK | ISTRIP);                     /* Parity checking ??*/
                        tio.c_cflag |= PARENB;                               /* Select Parity Enable through Control options*/
                        tio.c_cflag &= ~PARODD;                              /* Select the Even Parity through Control options*/
                        break;
                    case SerialPortSettings::PY_ODD:
                        tio.c_iflag |= (INPCK | ISTRIP);                     /* Parity checking ??*/
                        tio.c_cflag |= PARENB;                               /* Select Parity Enable through Control options*/
                        tio.c_cflag |= PARODD;                               /* Select the Odd Parity through Control options*/
                        break;
                    case SerialPortSettings::PY_SPACE:
                        tio.c_cflag &= ~PARENB;                              /* Space parity is the same as No parity*/
                        break;
                    default:
                        tio.c_cflag &= ~PARENB;
                        break;
                }
            }

            {
                tio.c_cc[VMIN] = 0;
//                size_t timeout = _parent->_settings.getRxTimeout() / 100;
                tio.c_cc[VTIME] = 0;
//                tio.c_cc[VTIME] = timeout;
//                tio.c_cc[VTIME] = (timeout == 0) ? 1 : timeout;
//                tio.c_cc[VTIME] = _parent->_settings.getRxTimeout();
            }

            if(setTermios(&tio) == false)
            {
                return false;
            }

            {
                struct serial_struct serial;
                int ret = ioctl(_handle, TIOCGSERIAL, &serial);
                if (ret != 0)
                {
                    return false;
                }
                serial.flags |= ASYNC_LOW_LATENCY;
                ret = ioctl(_handle, TIOCSSERIAL, &serial);
                if (ret != 0)
                {
                    return false;
                }
            }

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
        SerialPort::Err getSystemError(int systemErrorCode = -1) const
        {
            if (systemErrorCode == -1)
                systemErrorCode = errno;

            SerialPort::Err err;

            switch (systemErrorCode)
            {
                case 0:
                    err = SerialPort::ERR_SUCCESS;
                    break;
                case EPERM:
                    err = SerialPort::ERR_PERMISSION;
                    break;
                case ENOENT:
                    err = SerialPort::ERR_DEVICENOTFOUND;
                    break;
                case EBADE:
                    err = SerialPort::ERR_DESCRIPTOR;
                    break;
                case EIO:
                    err = SerialPort::ERR_IO;
                    break;
                default:
                    err = SerialPort::ERR_UNKNOWN;
                    break;
            }
            return err;
        }

    private:
        SerialPort* _parent = nullptr;
        int _handle = -1;
        struct termios _restoredTio;
    };
}
