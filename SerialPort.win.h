#pragma once

#include <string>

#include <Windows.h>

#include "SerialPort.h"

namespace Airmar200WXH
{
    class SerialPort::Sys
    {
    public:
        Sys(SerialPort* parent)
            : _parent(parent) {}

        HANDLE handle() const { return _handle; }

        bool isOpen() const
        {
            return (_handle != INVALID_HANDLE_VALUE);
        }

		bool openTty(std::string port)
        {
            DWORD desiredAccess = 0;
            desiredAccess |= GENERIC_READ;
            desiredAccess |= GENERIC_WRITE;
            //if(port.start)

            port = "\\\\.\\" + port;
            _handle = ::CreateFileA(reinterpret_cast<const char*>(port.c_str()), desiredAccess, 0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (_handle == INVALID_HANDLE_VALUE)
                return false;
            {
                DCB dcb;
                if (!getDcb(&dcb))
                    return false;

                _restoredDcb = dcb;

                dcb.fBinary = TRUE;
                dcb.fInX = FALSE;
                dcb.fOutX = FALSE;
                dcb.fAbortOnError = FALSE;
                dcb.fNull = FALSE;
                dcb.fErrorChar = FALSE;

    //            if (dcb.fDtrControl == DTR_CONTROL_HANDSHAKE)
    //                dcb.fDtrControl = DTR_CONTROL_DISABLE;
                dcb.fDtrControl = DTR_CONTROL_DISABLE;

				const DWORD BAUND_RATES[SerialPortSettings::BR_TOTAL] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
				if(uint32_t(_parent->_settings.baudRate) >= SerialPortSettings::BR_TOTAL)
					return false;
				dcb.BaudRate = BAUND_RATES[_parent->_settings.baudRate];

                switch (_parent->_settings.stopBits)
                {
                case SerialPortSettings::SB_ONE:
                    dcb.StopBits =  ONESTOPBIT;
                    break;
                case SerialPortSettings::SB_ONEANDHALF:
                    dcb.StopBits =  ONE5STOPBITS;
                    break;
                case SerialPortSettings::SB_TWO:
                    dcb.StopBits =  TWOSTOPBITS;
                    break;
                default:
                    dcb.StopBits =  ONESTOPBIT;
                    break;
                }

				const BYTE DATA_BITS[SerialPortSettings::DB_TOTAL] = {5, 6, 7, 8};
				if(uint32_t(_parent->_settings.dataBits) >= SerialPortSettings::DB_TOTAL)
					return false;
				dcb.ByteSize = DATA_BITS[_parent->_settings.dataBits];

                switch (_parent->_settings.parity)
                {
                case SerialPortSettings::Parity::PY_NO:
                    dcb.Parity = NOPARITY;
                    break;
                case SerialPortSettings::PY_EVEN:
                    dcb.Parity = static_cast<BYTE>(PARITY_EVEN);
                    break;
                case SerialPortSettings::PY_ODD:
                    dcb.Parity = static_cast<BYTE>(PARITY_ODD);
                    break;
                case SerialPortSettings::PY_SPACE:
                    dcb.Parity = static_cast<BYTE>(PARITY_SPACE);
                    break;
                default:
                    dcb.Parity = NOPARITY;
                    break;
                }
                dcb.fParity  = FALSE;

                if (!setDcb(&dcb))
                    return false;
            }

            {
                if (!::GetCommTimeouts(_handle, &_restoredCommTimeouts))
                {
                    _parent->setErr(getSystemError());
                    return false;
                }

                {
                    COMMTIMEOUTS currentCommTimeouts;
                    ::ZeroMemory(&currentCommTimeouts, sizeof(currentCommTimeouts));
//					if(_parent->_settings.getRxTimeout() > 0)
                    {
                        currentCommTimeouts.ReadIntervalTimeout = MAXDWORD;
						currentCommTimeouts.ReadTotalTimeoutConstant = 50;
//						currentCommTimeouts.ReadTotalTimeoutConstant = _parent->_settings.getRxTimeout();
                    }
//					else if(_parent->_settings.getRxTimeout() == 0)
//                    {
//                        currentCommTimeouts.ReadIntervalTimeout = MAXDWORD;
//                    }
//                    else
//                    {
//                        currentCommTimeouts.ReadTotalTimeoutConstant = MAXDWORD;
//                        currentCommTimeouts.ReadTotalTimeoutConstant = MAXDWORD;
//                    }
                    if (!::SetCommTimeouts(_handle, &currentCommTimeouts))
                    {
                        _parent->setErr(getSystemError());
                        return false;
                    }
                }
            }

            {
                DWORD originalEventMask = 0;
                originalEventMask |= EV_RXCHAR;
                if (!::SetCommMask(_handle, originalEventMask))
                {
                    _parent->setErr(getSystemError());
                    return false;
                }
            }

            return true;
        }

		void closeTty()
        {
            if(_handle != INVALID_HANDLE_VALUE)
            {
                setDcb(&_restoredDcb);
                if (!::SetCommTimeouts(_handle, &_restoredCommTimeouts))
                    _parent->setErr(getSystemError());
                if (!::CloseHandle(_handle))
                    _parent->setErr(getSystemError());
                _handle = INVALID_HANDLE_VALUE;
            }
        }

		int readTty(void* data, const size_t size, const size_t timeoutMSec)
		{
			COMMTIMEOUTS currentCommTimeouts;
			::ZeroMemory(&currentCommTimeouts, sizeof(currentCommTimeouts));
			currentCommTimeouts.ReadIntervalTimeout = MAXDWORD;
			currentCommTimeouts.ReadTotalTimeoutConstant = timeoutMSec;
			if (!::SetCommTimeouts(_handle, &currentCommTimeouts))
			{
				_parent->setErr(getSystemError());
				return -1;
			}

            DWORD numberOfBytesRead = 0;
            if (::ReadFile(_handle, data, size, &numberOfBytesRead, nullptr))
            {
                return static_cast<int>(numberOfBytesRead);
            }

            _parent->setErr(getSystemError());
            return -1;
        }

		int writeTty(const void* data, size_t size)
        {
            DWORD numberOfBytesWritten = 0;
            if (::WriteFile(_handle, data, size, &numberOfBytesWritten, nullptr))
            {
                return static_cast<int>(numberOfBytesWritten);
            }
            _parent->setErr(getSystemError());
            return -1;
        }

    private:
        SerialPort* _parent = nullptr;
        HANDLE _handle = INVALID_HANDLE_VALUE;
        DCB _restoredDcb;
        COMMTIMEOUTS _restoredCommTimeouts;

    private:
        SerialPort::Err getSystemError(int systemErrorCode = -1) const
        {
            if (systemErrorCode == -1)
                systemErrorCode = ::GetLastError();

            SerialPort::Err err;

            switch (systemErrorCode)
            {
            case ERROR_SUCCESS:
                err = SerialPort::ERR_SUCCESS;
                break;
            case ERROR_IO_PENDING:
                err = SerialPort::ERR_SUCCESS;
                break;
            case ERROR_MORE_DATA:
                err = SerialPort::ERR_SUCCESS;
                break;
            case ERROR_FILE_NOT_FOUND:
                err = SerialPort::ERR_DEVICENOTFOUND;
                break;
            case ERROR_PATH_NOT_FOUND:
                err = SerialPort::ERR_DEVICENOTFOUND;
                break;
            case ERROR_INVALID_NAME:
                err = SerialPort::ERR_DEVICENOTFOUND;
                break;
            case ERROR_ACCESS_DENIED:
                err = SerialPort::ERR_PERMISSION;
                break;
            case ERROR_INVALID_HANDLE:
                err = SerialPort::ERR_RESOURCE;
                break;
            case ERROR_INVALID_PARAMETER:
                err = SerialPort::ERR_UNSUPPORTEDOPERATION;
                break;
            case ERROR_BAD_COMMAND:
                err = SerialPort::ERR_RESOURCE;
                break;
            case ERROR_DEVICE_REMOVED:
                err = SerialPort::ERR_RESOURCE;
                break;
            case ERROR_OPERATION_ABORTED:
                err = SerialPort::ERR_RESOURCE;
                break;
            case WAIT_TIMEOUT:
                err = SerialPort::ERR_TIMEOUT;
                break;
            default:
                err = SerialPort::ERR_UNKNOWN;
                break;
            }
            return err;
        }

        bool getDcb(DCB *dcb)
        {
            ::ZeroMemory(dcb, sizeof(DCB));
            dcb->DCBlength = sizeof(DCB);

            if (!::GetCommState(_handle, dcb))
            {
                _parent->setErr(getSystemError());
                return false;
            }
            return true;
        }

        bool setDcb(DCB* dcb)
        {
			if (::SetCommState(_handle, dcb) == FALSE)
            {
                _parent->setErr(getSystemError());
                return false;
            }
            return true;
        }
    //public:
     //   static const speed_t & getBD(SerialPortSettings::BaudRate bd) { return BAUNDRATES[bd]; }

    private:
//        static constexpr speed_t BAUNDRATES[SerialPortSettings::BR_TOTAL] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
    };
}
