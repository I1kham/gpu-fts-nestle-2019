#include "CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer::passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
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
	
	if (rhea::string::strUTF16toUTF8(utf16_CPUMasterVersionString, buffer, sizeof(buffer)))
		server->sendAjaxAnwer(hClient, ajaxRequestID, buffer, rhea::string::utf8::lengthInBytes(buffer));
	else
	{
		DBGBREAK;
		sprintf_s((char*)buffer, sizeof(buffer), "ERR utf8 conversion");
		server->sendAjaxAnwer(hClient, ajaxRequestID, buffer, 1 + (u16)strlen((const char*)buffer));
	}
}

