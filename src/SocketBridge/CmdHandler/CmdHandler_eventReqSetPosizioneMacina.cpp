#include "CmdHandler_eventReqSetPosizioneMacina.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaNetBufferView.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqSetPosizioneMacina::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen)
{
    assert (payloadLen >= 2);

	rhea::NetStaticBufferViewR nbr;
	nbr.setup(&payload[1], payloadLen-1, rhea::eBigEndian);
	
	u8 macina_1o2 = 0;
	u16 target = 100;
	nbr.readU8(macina_1o2);
	nbr.readU16(target);
	cpubridge::ask_CPU_SET_POSIZIONE_MACINA(from, getHandlerID(), macina_1o2, target);
}

//***********************************************************
void CmdHandler_eventReqSetPosizioneMacina::onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
}
