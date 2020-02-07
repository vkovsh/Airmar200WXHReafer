#include "Airmar200WXHController.h"
#include <QDebug>
#include <QList>

using namespace Airmar200WXH;

Airmar200WXHController::Airmar200WXHController(const SerialPortSettings& serialPortSettings,
                                               const uint8_t TRIES_BEFORE_FAIL):
                                                _ioDev(serialPortSettings),
                                                TRIES_BEFORE_FAIL(TRIES_BEFORE_FAIL){};

Airmar200WXHController::~Airmar200WXHController()
{};

RetCode Airmar200WXHController::open()
{
    return _ioDev.open();
}

void Airmar200WXHController::close()
{
    _ioDev.close();
}

RetCode Airmar200WXHController::checkConnetion()
{
    RetCode ret = _ioDev.isOpen();
    if (ret == RetCode::RC_SUCCESS)
        return ret;
    ret = _ioDev.open();
    return ret;
}

RetCode Airmar200WXHController::readDataLoop()
{
//    qDebug() << _time._timer.elapsed();
    {
        RetCode ret = checkConnetion();
        if (ret != RetCode::RC_SUCCESS)
        return ret;
    }

    RetCode ret = RetCode::RC_ERR;

    QByteArray rxBuf;
    constexpr size_t DELAY_MSEC = 30;
    ret = _ioDev.recieve(rxBuf, DELAY_MSEC);
    if (ret == RetCode::RC_SUCCESS)
    {
//        qDebug() << rxBuf;
        QList<QByteArray> splittedRxBuf = rxBuf.split('\n');
        for (int cmdIndex = 0; cmdIndex < splittedRxBuf.size(); ++cmdIndex)
        {
            QByteArray cmdStr = splittedRxBuf.at(cmdIndex);
            QList<QByteArray> splittedCmdStr = cmdStr.split(',');
            if (splittedCmdStr.at(0) == "$GPGGA")
            {
                QByteArray timeStr = splittedCmdStr[1];
//                QByteArray timeStr = "140730.20";
                if ((false == timeStr.isEmpty()) && (timeStr.size() >= 6))
                {
//                    140730.20
                    const char* buf = timeStr.data();
                    qDebug() << buf;
                    _time.setTime((buf[0] - '0') * 10 + (buf[1] - '0'),
                            (buf[2] - '0') * 10 + (buf[3] - '0'),
                            (buf[4] - '0') * 10 + (buf[5] - '0'));
                    _time.resetTimer();
                    _time.setValidState(true);
                }
                else if (_time.elapsedMSecAfterUpdate() > 3000)
                {
                    _time.setValidState(false);
                }

//                qDebug() << timeStr;
            }
            else if (splittedCmdStr.at(0) == "$WIMDA")
            {

            }
        }
    }
    return ret;
}

RetCode Airmar200WXHController::readLocation(Airmar200WXHController::Location& loc)
{
    {
        RetCode ret = checkConnetion();
        if (ret != RetCode::RC_SUCCESS)
        return ret;
    }


//    QByteArray waitRxBuf;
//    if(val == true)
//        {
//            txBuf = "BZG CS 1\n";
//            waitRxBuf = "BZG CS 1 OK";
//        }
//        else
//        {
//            txBuf = "BZG CS 0\n";
//            waitRxBuf = "BZG CS 0 OK";
//        }

//        RetCode ret = RetCode::RC_ERR;
//        for (int i = 0; i < TRIES_BEFORE_FAIL; ++i)
//        {
//            {
//                ret = _ioDev.transmit(txBuf);
//                if (ret != RetCode::RC_SUCCESS)
//                    continue;
//            }

//            {
//                QByteArray rxBuf;
//                constexpr size_t SWITCH_CS = 10;
//                ret = _ioDev.recieve(rxBuf, SWITCH_CS);
//                if (ret != RetCode::RC_SUCCESS)
//                    continue;
//                if(rxBuf == waitRxBuf)
//                    return RetCode::RC_SUCCESS;
//                else
//                    ret = RetCode::RC_ERR_BAD_ANSWER;
//            }
//        }
//        return ret;

}
