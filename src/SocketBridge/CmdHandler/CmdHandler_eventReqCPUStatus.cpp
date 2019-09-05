#include "CmdHandler_eventReqCPUStatus.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqCPUStatus::handleRequest (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_QUERY_STATE (from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqCPUStatus::handleAnswer (socketbridge::Server *server, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 state[3];
	cpubridge::translateNotify_CPU_STATE_CHANGED (msgFromCPUBridge, &state[0], &state[1], &state[2]);

    server->sendEvent (hClient, EVENT_TYPE_FROM_SOCKETCLIENT, &state, 3);
}
