#include "CmdHandler_eventReqSimpleProgramCmd.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqSimpleProgramCmd::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload,  u16 payloadLen UNUSED_PARAM)
{
	assert(payloadLen > 1);
	const cpubridge::eCPUProgrammingCommand cmd = (cpubridge::eCPUProgrammingCommand)payload[1];
	cpubridge::ask_CPU_PROGRAMMING_CMD (from, getHandlerID(), cmd, NULL, 0);
}

//***********************************************************
void CmdHandler_eventReqSimpleProgramCmd::onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
	//quest fn non viene mai chiamata, il CPUBridge non risponde mai ad una richiesta di questo tipo
}
