#include "CmdHandler_ajaxReqGetPosizioneMacina.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u8		m;
};

//***********************************************************
bool ajaxReqGetPosizioneMacina_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "m") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
        if (h==1 || h==2)
			input->m = (u8)h;
		else
			input->m = 0;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReqGetPosizioneMacina::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqGetPosizioneMacina_jsonTrapFunction, &data))
	{
		if (data.m != 0)
			cpubridge::ask_CPU_GET_POSIZIONE_MACINA(from, getHandlerID(), data.m);
	}
}

//***********************************************************
void CmdHandler_ajaxReqGetPosizioneMacina::onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
	u8 macina = 0;
	u16 posizione = 0;
	cpubridge::translateNotify_CPU_POSIZIONE_MACINA(msgFromCPUBridge, &macina, &posizione);

	char resp[64];
	sprintf_s(resp, sizeof(resp), "{\"m\":%d,\"v\":%d}", macina, posizione);
	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));
}
