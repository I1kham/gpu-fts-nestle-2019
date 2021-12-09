#include "CmdHandler_ajaxReq_P0x32_ScivoloRotanteBrewmatic.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u8		perc;
};

//***********************************************************
bool CmdHandler_ajaxReq_P0x32_ScivoloRotanteBrewmatic_jsonTrapFunction(const u8* const fieldName, const u8* const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "perc") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
        if (h > 100)
			input->perc = 100;
		else
			input->perc = static_cast<u8>(h);
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReq_P0x32_ScivoloRotanteBrewmatic::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params)
{
	sInput data;
	if (rhea::json::parse(params, CmdHandler_ajaxReq_P0x32_ScivoloRotanteBrewmatic_jsonTrapFunction, &data))
	{
		cpubridge::ask_CPU_ATTIVAZIONE_SCIVOLO_BREWMATIC(from, getHandlerID(), (u8)data.perc);
	}
}

//***********************************************************
void CmdHandler_ajaxReq_P0x32_ScivoloRotanteBrewmatic::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
	char text[4] = { 'O', 'K', 0, 0 };
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)text, (u16)strlen(text));
}
