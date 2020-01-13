#include "CmdHandler_eventReq_P0x06_GetAllDecounters.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReq_P0x06_GetAllDecounters::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_ALL_DECOUNTER_VALUES (from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReq_P0x06_GetAllDecounters::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questo, modifica anche rhea::app::GetAllDecounters::decodeAnswer()
	u16 values[32];
	cpubridge::translateNotify_CPU_ALL_DECOUNTER_VALUES(msgFromCPUBridge, values);

	u8 buffer[32 * sizeof(u16)];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, sizeof(buffer), rhea::eBigEndian);
	for (u8 i = 0; i < 13; i++)
		nbw.writeU16(values[i]);

	server->sendEvent(hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, nbw.length());
}



