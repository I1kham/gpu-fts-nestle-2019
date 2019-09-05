#include "CmdHandler_eventReqSelStatus.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqSelStatus::handleRequest (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_QUERY_RUNNING_SEL_STATUS (from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqSelStatus::handleAnswer (socketbridge::Server *server, const rhea::thread::sMsg &msgFromCPUBridge)
{
	cpubridge::eRunningSelStatus s = cpubridge::eRunningSelStatus_finished_KO;
	cpubridge::translateNotify_CPU_RUNNING_SEL_STATUS (msgFromCPUBridge, &s);

	const u8 status = (u8)s;
    server->sendEvent (hClient, EVENT_TYPE_FROM_SOCKETCLIENT, &status, 1);
}
