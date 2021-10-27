#include "CmdHandler_ajaxReq_P0x23_startGrinderSpeedTest.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u8		macina1o2;
	u8		durataSec;
};

//***********************************************************
bool ajaxReqStartGrinderSpeedTest_jsonTrapFunction(const u8 *fieldName, const u8 *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "m") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
        if (h != 2)
			input->macina1o2 = 1;
		else
			input->macina1o2 = 2;
	}
	if (strcasecmp((const char*)fieldName, "d") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
        if (h <= 0xff)
			input->durataSec = (u8)h;
		else
			input->durataSec = 1;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReq_P0x23_startGrinderSpeedTest::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqStartGrinderSpeedTest_jsonTrapFunction, &data))
		cpubridge::ask_CPU_START_GRINDER_SPEED_TEST (from, getHandlerID(), data.macina1o2, data.durataSec);
}

//***********************************************************
void CmdHandler_ajaxReq_P0x23_startGrinderSpeedTest::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	bool bStarted = false;
	cpubridge::translateNotify_CPU_START_GRINDER_SPEED_TEST(msgFromCPUBridge, &bStarted);

	char text[4] = { 'O', 'K', 0, 0 };
	if (!bStarted)
	{
		text[0] = 'K';
		text[1] = 'O';
	}
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)text, (u16)strlen(text));
}
