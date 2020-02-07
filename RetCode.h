#pragma once

namespace Airmar200WXH
{
    class RetCode
    {
    public:
        enum RetCodeValue
        {
            RC_SUCCESS = 0,
            RC_ERR = 1,
            RC_ERR_PORT_IN_USE = 2,
            RC_ERR_NOT_OPENED = 3,
            RC_ERR_CANNOT_OPEN = 4,
            RC_ERR_READ_ERROR = 5,
            RC_ERR_TIMEOUT = 6,
            RC_ERR_NO_MSG = 7,
            RC_ERR_TOTAL = 8,
        };
    public:
        RetCode(){}
        RetCode(RetCodeValue retCode): _retCode(retCode){}
    public:
        const RetCodeValue & getCode() const { return _retCode; }

        void setCode(const RetCodeValue& retCode) { _retCode = retCode; }
        int toInt() const { return int(_retCode); }
    public:
        bool operator==(const RetCode& rhs) const { return _retCode == rhs.getCode(); }
        bool operator==(const RetCodeValue & retCode) const { return _retCode == retCode; }

        bool operator!=(const RetCode& rhs) const { return _retCode != rhs.getCode(); }
        bool operator!=(const RetCodeValue& retCode) const { return _retCode != retCode; }
    private:
        RetCodeValue _retCode = RC_SUCCESS;
    };
}
