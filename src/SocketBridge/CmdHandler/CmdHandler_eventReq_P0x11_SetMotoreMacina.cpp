#include "CmdHandler_eventReq_P0x11_SetMotoreMacina.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReq_P0x11_SetMotoreMacina::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen)
{
    assert (payloadLen >= 2);
    const u8 macina_1to4 = payload[1];
	const cpubridge::eCPUProg_macinaMove m = (cpubridge::eCPUProg_macinaMove)payload[2];

	cpubridge::ask_CPU_SET_MOTORE_MACINA_AA (from, getHandlerID(), macina_1to4, m);
}

//***********************************************************
void CmdHandler_eventReq_P0x11_SetMotoreMacina::onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questa, modifica anche rhea::app::SetMotoreMacina::decodeAnswer()

	//rispondo con:
	//  1 byte per il numero macina
	//  1 byte per il tipo di movimento
	u8 macina_1to4 = 0;
	cpubridge::eCPUProg_macinaMove m;
	cpubridge::translateNotify_CPU_MOTORE_MACINA (msgFromCPUBridge, &macina_1to4, &m);


	u8 buffer[2];
	buffer[0] = macina_1to4;
	buffer[1] = (u8)m;
	server->sendEvent (hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, 2);
}
