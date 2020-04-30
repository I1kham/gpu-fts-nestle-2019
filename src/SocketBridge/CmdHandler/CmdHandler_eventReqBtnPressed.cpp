#include "CmdHandler_eventReqBtnPressed.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqBtnPressed::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen)
{
    assert (payloadLen > 1);
    const u8 btnNum = payload[1];

	cpubridge::ask_CPU_SEND_BUTTON (from, btnNum);
}

//***********************************************************
void CmdHandler_eventReqBtnPressed::onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &h UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
    //quest fn non viene mai chiamata, CPUBridge non risponde mai ad una richiesta di questo tipo
}
