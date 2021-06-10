#include "CmdHandler_eventReqStartSelForceJug.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqStartSelForceJug::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen)
{
    assert (payloadLen > 1);
    const u8 selNum = payload[1];

	cpubridge::ask_CPU_START_SELECTION_AND_FORCE_JUG (from, selNum);
}

//***********************************************************
void CmdHandler_eventReqStartSelForceJug::onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &h UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
    //quest fn non viene mai chiamata, il CPUBridge non risponde mai ad una richiesta di questo tipo
}
