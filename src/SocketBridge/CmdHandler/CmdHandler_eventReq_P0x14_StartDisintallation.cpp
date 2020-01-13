#include "CmdHandler_eventReq_P0x14_StartDisintallation.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReq_P0x14_StartDisintallation::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_DISINTALLAZIONE(from);
}

//***********************************************************
void CmdHandler_eventReq_P0x14_StartDisintallation::onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &h UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
    //quest fn non viene mai chiamata, il CPUBridge non risponde mai ad una richiesta di questo tipo
}
