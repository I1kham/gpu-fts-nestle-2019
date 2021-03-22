#include "CmdHandler_ajaxReqJugRepetitions.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqJugRepetitions::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_JUG_REPETITIONS(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqJugRepetitions::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8	len;
	u8	buffer[NUM_MAX_SELECTIONS + 1];

	memset(buffer, 0, sizeof buffer);
	cpubridge::translateNotify_CPU_GET_JUG_REPETITIONS (msgFromCPUBridge, &len, buffer, (sizeof buffer) - 1);

	char resp[NUM_MAX_SELECTIONS + 30];
	sprintf_s(resp, sizeof(resp), "{\"n\":%d,\"s\":\"%s\"}", len, buffer);
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
