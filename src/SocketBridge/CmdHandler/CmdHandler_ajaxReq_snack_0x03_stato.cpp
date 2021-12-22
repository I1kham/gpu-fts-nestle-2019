#include "CmdHandler_ajaxReq_snack_0x03_stato.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_snack_0x03_stato::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_SNACK_GET_STATUS(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_snack_0x03_stato::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	bool bIsAlive;
	u8 selStatus1_48[6];
	cpubridge::translateNotify_SNACK_GET_STATUS(msgFromCPUBridge, &bIsAlive, selStatus1_48, sizeof(selStatus1_48));
	

	//preparo un array di NUM_MAX_SELECTIONS char usando '0' per indicare una sel non disponibile e '1' per quelle disponibili
	static const u8 NUM_SNACK_SEL = 48;
	char avail[NUM_SNACK_SEL+1];
    for (u8 i=0; i< NUM_SNACK_SEL; i++)
    {
		const u8 byte = i / 8;
		const u8 bit  = i % 8;
		if ( (selStatus1_48[byte] & (0x01 << bit)) == 0)
            avail[i] = '0';
		else
			avail[i] = '1';
    }
    avail[NUM_SNACK_SEL] = 0x00;


    char resp[256];
	sprintf_s(resp, sizeof(resp), "{\"state\":\"%d\",\"n\":%d,\"avail\":\"%s\"}", bIsAlive?1:0, NUM_SNACK_SEL, avail);
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
