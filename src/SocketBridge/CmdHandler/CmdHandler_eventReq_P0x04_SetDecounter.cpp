#include "CmdHandler_eventReq_P0x04_SetDecounter.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReq_P0x04_SetDecounter::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	assert(payloadLen >= 4);
	rhea::NetStaticBufferViewR nbr;
	nbr.setup(&payload[1], payloadLen - 1, rhea::eEndianess::eBigEndian);

	u8 what;
	nbr.readU8(what);
	
	u16 value = 0;
	nbr.readU16(value);
	
	cpubridge::ask_CPU_SET_DECOUNTER (from, getHandlerID(), (cpubridge::eCPUProg_decounter)what, value);
}

//***********************************************************
void CmdHandler_eventReq_P0x04_SetDecounter::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questo, modifica anche rhea::app::SetDecounter::decodeAnswer()
	cpubridge::eCPUProg_decounter which = cpubridge::eCPUProg_decounter::unknown;
	u16 value = 0;
	cpubridge::translateNotify_CPU_DECOUNTER_SET (msgFromCPUBridge, &which, &value);


	u8 buffer[4];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, sizeof(buffer), rhea::eEndianess::eBigEndian);
	nbw.writeU8((u8)which);
	nbw.writeU16(value);
	server->sendEvent(hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, nbw.length());
}



