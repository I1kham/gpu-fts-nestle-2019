#include "CmdHandler_eventReqSelAvailability.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqSelAvailability::handleRequest (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_QUERY_SEL_AVAIL(from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqSelAvailability::handleAnswer (socketbridge::Server *server, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//rispondo con:
	//  1 byte per il num sel
	//  1 byte per ogni sel, dove 0x00 =sel non attiva, 0x01=sel attiva

	cpubridge::sCPUSelAvailability selAvail;
	cpubridge::translateNotify_CPU_SEL_AVAIL_CHANGED(msgFromCPUBridge, &selAvail);

	u8 buffer[NUM_MAX_SELECTIONS + 1];
	buffer[0] = NUM_MAX_SELECTIONS;
	for (u8 i = 1; i <= NUM_MAX_SELECTIONS; i++)
	{
		if (selAvail.isAvail(i + 1))
			buffer[i] = 0x01;
		else
			buffer[i] = 0x00;
	}
	
    server->sendEvent (hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, sizeof(buffer));
}


