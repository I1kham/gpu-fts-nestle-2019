#include "CmdHandler_eventReq_P0x03_CPUSanWashingStatus.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReq_P0x03_CPUSanWashingStatus::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_PROGRAMMING_CMD_QUERY_SANWASH_STATUS(from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReq_P0x03_CPUSanWashingStatus::onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
	//NB: se modifichi questo, modifica anche rhea::app::SanWashingStatus::decodeAnswer()

	u8 buffer[16] = { 0,0,0,0 };
	cpubridge::translateNotify_SAN_WASHING_STATUS(msgFromCPUBridge, &buffer[0], &buffer[1], &buffer[2], &buffer[3]);
	server->sendEvent(hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, 11);
}
