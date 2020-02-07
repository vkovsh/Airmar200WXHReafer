#include "Airmar200WXHController.h"

using namespace Airmar200WXH;

int	main()
{
    RetCode ret = RetCode::RC_ERR;
    Airmar200WXHController ctrl(SerialPortSettings("ttyUSB0", SerialPortSettings::BR_4800, 1), 1);
    ret = ctrl.open();
    if (ret == RetCode::RC_SUCCESS)
    {
        while (1)
        {
            ctrl.readDataLoop();
        }
    }
	return 0;
}
