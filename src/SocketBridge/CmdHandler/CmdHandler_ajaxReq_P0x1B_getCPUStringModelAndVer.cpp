#include "CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaUTF8.h"
#include "../../rheaCommonLib/rheaUTF16.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer::passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_STRING_VERSION_AND_MODEL(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer::onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 utf16_CPUMasterVersionString[64];
	memset (utf16_CPUMasterVersionString, 0, sizeof(utf16_CPUMasterVersionString));
	cpubridge::translateNotify_CPU_STRING_VERSION_AND_MODEL(msgFromCPUBridge, utf16_CPUMasterVersionString, sizeof(utf16_CPUMasterVersionString));


	u8 buffer[256];
	u32 sizeOfBuffer = sizeof(buffer);
	if (rhea::utf16::toUTF8(utf16_CPUMasterVersionString, buffer, &sizeOfBuffer))
		server->sendAjaxAnwer(hClient, ajaxRequestID, (const char*)buffer, sizeOfBuffer);
	else
	{
		DBGBREAK;
		sprintf_s((char*)buffer, sizeof(buffer), "ERR utf8 conversion");
		server->sendAjaxAnwer(hClient, ajaxRequestID, (const char*)buffer, 1 + (u16)strlen((const char*)buffer));
	}
}

