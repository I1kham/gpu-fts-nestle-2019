#include "CmdHandler_ajaxReq_isQuickMenuPinCodeSet.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;



//***********************************************************
void CmdHandler_ajaxReq_isQuickMenuPinCodeSet::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_IS_QUICK_MENU_PINCODE_SET(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_isQuickMenuPinCodeSet::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	bool bYes;
	cpubridge::translateNotify_CPU_IS_QUICK_MENU_PINCODE_SET(msgFromCPUBridge, &bYes);
    
    char resp[8];
	if (bYes)
		sprintf_s (resp, sizeof(resp), "Y");
	else
		sprintf_s (resp, sizeof(resp), "N");
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}

