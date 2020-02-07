#pragma once

#include <string>

namespace Airmar200WXH
{
    struct SerialPortSettings
    {
    public:
        enum BaudRate
        {
            BR_UNKNOWN = -1,
            BR_1200 = 0,
            BR_2400 = 1,
            BR_4800 = 2,
            BR_9600 = 3,
            BR_19200 = 4,
            BR_38400 = 5,
            BR_57600 = 6,
            BR_115200 = 7,
            BR_TOTAL = 8,
        };

        enum DataBits
        {
            DB_UNKNOWN = -1,
            DB_5 = 0,
            DB_6 = 1,
            DB_7 = 2,
            DB_8 = 3,
            DB_TOTAL = 4,
        };

        enum Parity
        {
            PY_UNKNOWN = -1,
            PY_NO = 0,
            PY_EVEN = 2,
            PY_ODD = 3,
            PY_SPACE = 4,
            PY_MARK = 5,
        };

        enum StopBits
        {
            SB_UNKNOWN = -1,
            SB_ONE = 1,
            SB_ONEANDHALF = 3,
            SB_TWO = 2,
        };

        enum FlowControl
        {
            FC_UNKNOWN = -1,
            FC_NO,
            FC_HARDWARE,
            FC_SOFTWARE,
        };

    public:
        std::string port;
        BaudRate baudRate;
        DataBits dataBits = DB_8;
        Parity parity = PY_NO;
        StopBits stopBits = SB_ONE;
        FlowControl flowControl = FC_NO;
		const uint8_t rxTimeoutMaxCount;
//	private:
//		size_t _rxTimeout;
//	public:
//		size_t getRxTimeout() const { return _rxTimeout; }
    public:
        SerialPortSettings(std::string port, BaudRate baudRate, uint8_t rxTimeoutMaxCount)
            : port(port)
			, baudRate(baudRate)
			, rxTimeoutMaxCount(rxTimeoutMaxCount)
        {
        }
    };

    class SerialPort
    {
        SerialPort(const SerialPort&);

    private:
        class Sys;
    public:
        enum Err
        {
            ERR_SUCCESS = 0,
            ERR_DEVICENOTFOUND,
            ERR_PERMISSION,
            ERR_OPEN,
            ERR_PARITY,
            ERR_FRAMINGRROR,
            ERR_BREAKCONDITION,
            ERR_WRITE,
            ERR_READ,
            ERR_IO,
            ERR_RESOURCE,
            ERR_UNSUPPORTEDOPERATION,
            ERR_UNKNOWN,
            ERR_TIMEOUT,
            ERR_DESCRIPTOR,
            ERR_NOTOPEN
        };
    public:
        explicit SerialPort(const SerialPortSettings& settings);
        virtual ~SerialPort();
    public:
        const SerialPortSettings& settings() const { return _settings; }

        SerialPort::Err err() const { return _err; }
        void clearErr();

        bool isOpen() const;

        bool openTty();
        void closeTty();

        int writeTty(const void* data, const size_t size);
        int writeTty(const char* data);

        int readTty(void* data, const size_t size, const size_t timeout);

    private:
        Err _err;
        Sys* _sys = nullptr;
        SerialPortSettings _settings;
    private:
        void setErr(Err err);
    };
}
