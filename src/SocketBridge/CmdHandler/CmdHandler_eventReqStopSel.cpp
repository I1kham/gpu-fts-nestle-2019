#include "CmdHandler_eventReqStopSel.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqStopSel::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_STOP_SELECTION(from);
}

//***********************************************************
void CmdHandler_eventReqStopSel::onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
    //quest fn non viene mai chiamata, il CPUBridge non risponde mai ad una richiesta di questo tipo
}
