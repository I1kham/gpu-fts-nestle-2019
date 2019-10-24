#include "CmdHandler.h"

using namespace socketbridge;

//***************************************************
CmdHandler::CmdHandler(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec)
{
    this->handlerID = handlerID;

    if (dieAfterHowManyMSec == u64MAX)
        timeToDieMSec = u64MAX;
    else
        timeToDieMSec = rhea::getTimeNowMSec() + dieAfterHowManyMSec;

    this->identifiedClientHandle = identifiedClientHandle;
}


