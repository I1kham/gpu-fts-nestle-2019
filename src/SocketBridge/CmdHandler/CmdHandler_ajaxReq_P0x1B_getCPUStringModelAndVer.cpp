#include "CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer::passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_STRING_VERSION_AND_MODEL(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer::onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	bool isUnicode = false;
	u8 strCPUModelAndVer[68];
	memset (strCPUModelAndVer, 0, sizeof(strCPUModelAndVer));
	cpubridge::translateNotify_CPU_STRING_VERSION_AND_MODEL(msgFromCPUBridge, &isUnicode, strCPUModelAndVer, sizeof(strCPUModelAndVer));


	u8 buffer[80];
	u8 n = 0;
	if (isUnicode)
	{
		//anche se stiamo parlando di unicode, in realtà la stringa riportante il nome del master è sempre in inglese, quindi usa caratteri ascii.
		//Ignoro il byte alto e compongo la stringa come se fosse un ascii normale
		u8 i = 0;
		while (i < 128)
		{
			const u8 b1 = strCPUModelAndVer[i++];
			const u8 b2 = strCPUModelAndVer[i++];

			buffer[n++] = b2;
			if (b1 == 0x00 && b2 == 0x00)
				break;
		}
	}
	else
	{
		n = (u8)strlen((const char*)strCPUModelAndVer);
		memcpy(buffer, strCPUModelAndVer, n);
		buffer[n++] = 0x00;
	}

	server->sendAjaxAnwer(hClient, ajaxRequestID, (const char*)buffer, n);
}

