#include "CmdHandler_ajaxReqSelAvailability.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqSelAvailability::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_QUERY_SEL_AVAIL (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqSelAvailability::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	cpubridge::sCPUSelAvailability selAvail;
	cpubridge::translateNotify_CPU_SEL_AVAIL_CHANGED (msgFromCPUBridge, &selAvail);
    
	//preparo un array di NUM_MAX_SELECTIONS char usando '0' per indicare una sel non disponibile e '1' per quelle disponibili
	char avail[NUM_MAX_SELECTIONS+1];
    for (u8 i=0; i< NUM_MAX_SELECTIONS; i++)
    {
		if (selAvail.isAvail(i+1))
            avail[i] = '1';
        else
            avail[i] = '0';
    }
    avail[NUM_MAX_SELECTIONS] = 0x00;


    char resp[256];
	sprintf_s(resp, sizeof(resp), "{\"n\":%d,\"s\":\"%s\"}", NUM_MAX_SELECTIONS, avail);
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
