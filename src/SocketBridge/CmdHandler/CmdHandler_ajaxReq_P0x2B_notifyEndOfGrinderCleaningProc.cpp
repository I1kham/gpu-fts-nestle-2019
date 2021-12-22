#include "CmdHandler_ajaxReq_P0x2B_notifyEndOfGrinderCleaningProc.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
    u8		macinaNumDa1AN;
};

//***********************************************************
bool CmdHandler_ajaxReq_P0x2B_notifyEndOfGrinderCleaningProc_jsonTrapFunction(const u8 *const fieldName, const u8 *const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "m") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
        if (h < 0XFF)
            input->macinaNumDa1AN = static_cast<u8>(h);
		else
            input->macinaNumDa1AN = 1;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReq_P0x2B_notifyEndOfGrinderCleaningProc::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params)
{
	sInput data;
	if (rhea::json::parse(params, CmdHandler_ajaxReq_P0x2B_notifyEndOfGrinderCleaningProc_jsonTrapFunction, &data))
        cpubridge::ask_END_OF_GRINDER_CLEANING_PROCEDURE (from, getHandlerID(), data.macinaNumDa1AN);
}

//***********************************************************
void CmdHandler_ajaxReq_P0x2B_notifyEndOfGrinderCleaningProc::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
	char text[4] = { 'O', 'K', 0, 0 };
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)text, (u16)strlen(text));
}
