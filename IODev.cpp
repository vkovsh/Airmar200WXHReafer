#include <QString>

#include "IODev.h"

#include <QDebug>

using namespace Airmar200WXH;

IODev::IODev(const SerialPortSettings& serialPortSettings)
    : _dev(serialPortSettings)
{}

IODev::~IODev()
{}

RetCode IODev::open()
{
    bool ret = _dev.openTty();
    if(ret == false)
       return RetCode::RC_ERR_CANNOT_OPEN;
    return RetCode::RC_SUCCESS;
}

void IODev::close()
{
    _dev.closeTty();
}

RetCode IODev::recieve(QByteArray& buf, size_t timeout)
{
    buf.clear();
    const int BUFF_SIZE = 1024;
    char buffer[BUFF_SIZE];
    memset(buffer, 0, sizeof(buffer));
    const int MAX_SIZE = sizeof(buffer) - 1;
    int byteRecieved = 0;
    int leftTimeoutsCount = _dev.settings().rxTimeoutMaxCount;
    while (byteRecieved < BUFF_SIZE && leftTimeoutsCount != 0)
    {
        if(leftTimeoutsCount == 0)
        {
            return RetCode::RC_ERR_TIMEOUT;
        }

        int recv = -1;
        recv = _dev.readTty(buffer + byteRecieved, MAX_SIZE - byteRecieved, timeout);

//        qDebug() << buffer;
        if (recv == 0)
        {
            leftTimeoutsCount--;
            continue;
        }
        if (recv < 0)
        {
            return RetCode::RC_ERR_READ_ERROR;
        }

        byteRecieved += recv;
        {
            char* startLinePosPtr = NULL;
            startLinePosPtr = strchr(buffer, '$');
            if (startLinePosPtr == NULL)
            {
                return RetCode::RC_ERR_NO_MSG;
            }
            char* newLinePosPtr = NULL;
            if ((newLinePosPtr = strrchr(startLinePosPtr, '\n')) != NULL)
            {
                size_t newlinePos = newLinePosPtr - startLinePosPtr;
                size_t endOfLinePos = newlinePos - 1;
                if (startLinePosPtr[endOfLinePos] == '\r')
                    endOfLinePos--;
                const size_t bytesLineCount = endOfLinePos + 1;
                buf.append(startLinePosPtr, bytesLineCount);
//                qDebug() << buf;
                return RetCode::RC_SUCCESS;
            }
        }
    }
    return RetCode::RC_ERR;
}

RetCode IODev::isOpen() const
{
    return (_dev.isOpen() == true) ? RetCode::RC_SUCCESS : RetCode::RC_ERR_NOT_OPENED;
}
