#include "CmdHandler_ajaxReq_P0x25_caffeCortesia.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u8		m;
};

//***********************************************************
bool ajaxReq_P0x25_caffeCortesia_jsonTrapFunction(const u8 *const fieldName, const u8 *const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "m") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
			input->m = (u8)h;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReq_P0x25_caffeCortesia::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params)
{
	sInput data;
	data.m = 0;

	if (rhea::json::parse(params, ajaxReq_P0x25_caffeCortesia_jsonTrapFunction, &data))
	{
		if (data.m != 0)
			cpubridge::ask_CPU_RUN_CAFFE_CORTESIA(from, getHandlerID(), data.m);
	}
}

//***********************************************************
void CmdHandler_ajaxReq_P0x25_caffeCortesia::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	char resp[8];
	sprintf_s(resp, sizeof(resp), "OK");
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
