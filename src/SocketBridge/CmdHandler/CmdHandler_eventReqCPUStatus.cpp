#include "CmdHandler_eventReqCPUStatus.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqCPUStatus::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_QUERY_STATE (from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqCPUStatus::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questo, modifica anche rhea::app::CurrentCPUStatus::decodeAnswer()
	cpubridge::eVMCState vmcState;
	u8 state[3];
	cpubridge::translateNotify_CPU_STATE_CHANGED (msgFromCPUBridge, &vmcState, &state[1], &state[2]);

	state[0] = (u8)vmcState;
    server->sendEvent (hClient, EVENT_TYPE_FROM_SOCKETCLIENT, &state, 3);
}
