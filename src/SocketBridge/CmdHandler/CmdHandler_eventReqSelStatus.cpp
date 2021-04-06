#include "CmdHandler_eventReqSelStatus.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqSelStatus::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_QUERY_RUNNING_SEL_STATUS (from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqSelStatus::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questo, modifica anche rhea::app::CurrentSelectionRunningStatus::decodeAnswer()

	cpubridge::eRunningSelStatus s = cpubridge::eRunningSelStatus::finished_KO;
	cpubridge::translateNotify_CPU_RUNNING_SEL_STATUS (msgFromCPUBridge, &s);

	const u8 status = (u8)s;
    server->sendEvent (hClient, EVENT_TYPE_FROM_SOCKETCLIENT, &status, 1);
}
