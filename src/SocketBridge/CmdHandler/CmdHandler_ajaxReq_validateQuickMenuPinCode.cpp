#include "CmdHandler_ajaxReq_validateQuickMenuPinCode.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u16 pin;
};

//***********************************************************
bool  CmdHandler_ajaxReq_validateQuickMenuPinCode_jsonTrapFunction(const u8* const fieldName, const u8* const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "pin") == 0)
	{
		const u32 n = rhea::string::ansi::toU32((const char*)fieldValue);
		if (n <= 0xFFFF)
			input->pin = (u16)n;
		else
			input->pin = 0;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReq_validateQuickMenuPinCode::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	sInput data;
	if (rhea::json::parse(params, CmdHandler_ajaxReq_validateQuickMenuPinCode_jsonTrapFunction, &data))
	{
		cpubridge::ask_CPU_VALIDATE_QUICK_MENU_PINCODE(from, getHandlerID(), data.pin);
	}	
}

//***********************************************************
void CmdHandler_ajaxReq_validateQuickMenuPinCode::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	bool bAccepted;
	cpubridge::translateNotify_CPU_VALIDATE_QUICK_MENU_PINCODE(msgFromCPUBridge, &bAccepted);
    
    char resp[8];
	if (bAccepted)
		sprintf_s (resp, sizeof(resp), "OK");
	else
		sprintf_s (resp, sizeof(resp), "KO");
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}

