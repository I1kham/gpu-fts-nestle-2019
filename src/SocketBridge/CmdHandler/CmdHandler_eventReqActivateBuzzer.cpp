#include "CmdHandler_eventReqActivateBuzzer.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqActivateBuzzer::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen)
{
    assert (payloadLen > 1);
    const u8 numRepeat = payload[1];
    const u8 beepLen_dSec  = payload[2];
    const u8 pausaTraUnBeepELAltro_dSec  = payload[3];
	cpubridge::ask_CPU_ACTIVATE_BUZZER (from, getHandlerID(), numRepeat, beepLen_dSec, pausaTraUnBeepELAltro_dSec);
}

//***********************************************************
void CmdHandler_eventReqActivateBuzzer::onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &h UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
    //quest fn non viene mai chiamata, CPUBridge non risponde mai ad una richiesta di questo tipo
}
