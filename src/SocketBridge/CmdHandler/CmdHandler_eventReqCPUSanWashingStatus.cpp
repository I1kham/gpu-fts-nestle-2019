#include "CmdHandler_eventReqCPUSanWashingStatus.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqCPUSanWashingStatus::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_PROGRAMMING_CMD_QUERY_SANWASH_STATUS(from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqCPUSanWashingStatus::onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
	//NB: se modifichi questo, modifica anche rhea::app::SanWashingStatus::decodeAnswer()

	u8 buffer[4] = { 0,0,0,0 };
	cpubridge::translateNotify_SAN_WASHING_STATUS(msgFromCPUBridge, &buffer[0], &buffer[1], &buffer[2]);
	server->sendEvent(hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, 3);
}
