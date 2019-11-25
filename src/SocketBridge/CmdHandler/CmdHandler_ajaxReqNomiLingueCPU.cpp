#include "CmdHandler_ajaxReqNomiLingueCPU.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqNomiLingueCPU::passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_NOMI_LINGE_CPU (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqNomiLingueCPU::onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 strLingua1UTF16[20];
	u16 strLingua2UTF16[20];
	memset (strLingua1UTF16, 0, sizeof(strLingua1UTF16));
	memset(strLingua2UTF16, 0, sizeof(strLingua2UTF16));
	cpubridge::translateNotify_NOMI_LINGE_CPU(msgFromCPUBridge, strLingua1UTF16, strLingua2UTF16);


	u8 buffer[32 * 2 + 4];
	u8 n = 0;
	for (u8 i = 0;i < 20; i++)
	{
		buffer[n++] = (u8)((strLingua1UTF16[i] & 0xFF00) >> 8);
		buffer[n++] = (u8)(strLingua1UTF16[i] & 0x00FF);
		if (strLingua1UTF16[i] == 0x0000)
			break;
	}

	for (u8 i = 0;i < 20; i++)
	{
		buffer[n++] = (u8)((strLingua2UTF16[i] & 0xFF00) >> 8);
		buffer[n++] = (u8)(strLingua2UTF16[i] & 0x00FF);
		if (strLingua2UTF16[i] == 0x0000)
			break;
	}
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const char*)buffer, n);
}

