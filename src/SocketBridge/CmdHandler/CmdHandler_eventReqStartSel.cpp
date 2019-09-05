#include "CmdHandler_eventReqStartSel.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqStartSel::handleRequest (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen)
{
    assert (payloadLen > 1);
    const u8 selNum = payload[1];

	cpubridge::ask_CPU_START_SELECTION (from, selNum);
}

//***********************************************************
void CmdHandler_eventReqStartSel::handleAnswer (socketbridge::Server *server UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
    //quest fn non viene mai chiamata, il server non risponde mai ad una richiesta di questo tipo
}
