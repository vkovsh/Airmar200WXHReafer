#pragma once

#include <string>
#include <netinet/in.h>

namespace Airmar200WXH
{
    struct SocketClientSettings
    {
    public:
        int16_t sin_family;
        uint16_t sin_port;
        std::string sin_addr;

    public:
        SocketClientSettings(const short sin_family,
                             unsigned short sin_port,
                             const char* sin_addr)
            : sin_family(sin_family),
              sin_port(sin_port),
              sin_addr(sin_addr)
        {
        }
    };

    class Socket
    {
        Socket(const Socket& rhs);

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
        explicit Socket(const SocketClientSettings& settings);
        virtual ~Socket();
    public:
        const SocketClientSettings& settings() const { return _settings; }

        Socket::Err err() const { return _err; }
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
        SocketClientSettings _settings;
        struct sockaddr_in _servAddr;
    private:
        void setErr(Err err);
    };
}
