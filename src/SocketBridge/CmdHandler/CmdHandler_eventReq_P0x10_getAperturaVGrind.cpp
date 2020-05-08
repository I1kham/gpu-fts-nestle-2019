#include "CmdHandler_eventReq_P0x10_getAperturaVGrind.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReq_P0x10_getAperturaVGrind::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen)
{
    assert (payloadLen >= 1);
    const u8 macina_1o2 = payload[1];

	cpubridge::ask_CPU_GET_POSIZIONE_MACINA(from, getHandlerID(), macina_1o2);
}

//***********************************************************
void CmdHandler_eventReq_P0x10_getAperturaVGrind::onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questa, modifica anche rhea::app::GetAperturaVgrind::decodeAnswer()

	//rispondo con:
	//  1 byte per il numero macina
	//  1 byte per il tipo di movimento
	u8 macina_1o2 = 0;
	u16 pos = 0;
	cpubridge::translateNotify_CPU_POSIZIONE_MACINA(msgFromCPUBridge, &macina_1o2, &pos);


	u8 buffer[4];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, sizeof(buffer), rhea::eBigEndian);
	nbw.writeU16(pos);
	nbw.writeU8(macina_1o2);
	server->sendEvent(hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, nbw.length());
}
