#include "CmdHandler_ajaxReq_P0x13_NomiLingueCPU.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x13_NomiLingueCPU::passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_NOMI_LINGE_CPU (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x13_NomiLingueCPU::onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 strLingua1UTF16[33];
	u16 strLingua2UTF16[33];
	memset (strLingua1UTF16, 0, sizeof(strLingua1UTF16));
	memset(strLingua2UTF16, 0, sizeof(strLingua2UTF16));
	cpubridge::translateNotify_NOMI_LINGE_CPU(msgFromCPUBridge, strLingua1UTF16, strLingua2UTF16);


	u8 buffer[16+(32+1) * 4];
	u8 n = 0;

	//quest 5 byte indicano che la codifica a seguire e' UTF16 invece di UTf8 che e' il default
	buffer[n++] = 0x01;
	buffer[n++] = 0x02;
	buffer[n++] = 0x03;
	buffer[n++] = 0x04;
	buffer[n++] = 0x05;


	for (u8 i = 0;i < 33; i++)
	{
		buffer[n++] = (u8)(strLingua1UTF16[i] & 0x00FF);
		buffer[n++] = (u8)((strLingua1UTF16[i] & 0xFF00) >> 8);
		if (strLingua1UTF16[i] == 0x0000)
			break;
	}
	buffer[n++] = '#';
	buffer[n++] = 0;

	for (u8 i = 0;i < 33; i++)
	{
		buffer[n++] = (u8)(strLingua2UTF16[i] & 0x00FF);
		buffer[n++] = (u8)((strLingua2UTF16[i] & 0xFF00) >> 8);
		if (strLingua2UTF16[i] == 0x0000)
			break;
	}
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const char*)buffer, n);
}

