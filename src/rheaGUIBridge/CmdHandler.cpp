#include "CmdHandler.h"

using namespace guibridge;

//***************************************************
CmdHandler::CmdHandler(const HWebsokClient &h, u16 handlerID, u64 dieAfterHowManyMSec)
{
    this->handlerID = handlerID;

    if (dieAfterHowManyMSec == u64MAX)
        timeToDieMSec = u64MAX;
    else
        timeToDieMSec = OS_getTimeNowMSec() + dieAfterHowManyMSec;

    hClient = h;
}


