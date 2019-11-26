#include "CmdHandler_eventReqRecalcFasciaOrariaFV.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqRecalcFasciaOrariaFV::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_RICARICA_FASCIA_ORARIA_FREEVEND(from);
}

//***********************************************************
void CmdHandler_eventReqRecalcFasciaOrariaFV::onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &h UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
    //quest fn non viene mai chiamata, il CPUBridge non risponde mai ad una richiesta di questo tipo
}
