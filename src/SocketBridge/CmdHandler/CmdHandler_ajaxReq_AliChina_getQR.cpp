#include "CmdHandler_ajaxReq_AliChina_getQR.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u8		name[128];
	u8		selNum;
	char	price[16];
};

//***********************************************************
bool ajaxReq_AliChina_getQR_jsonTrapFunction(const u8* const fieldName, const u8* const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "name") == 0)
	{
		rhea::string::utf8::copyStr (input->name, sizeof(input->name), fieldValue);
	}
	else if (strcasecmp((const char*)fieldName, "selNum") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
		if (h > 0 && h < 0xff)
			input->selNum = (u8)h;
		else
			input->selNum = 0xff;
	}	
	else if (strcasecmp((const char*)fieldName, "price") == 0)
	{
		rhea::string::utf8::copyStr ((u8*)input->price, sizeof(input->price), fieldValue);
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReq_AliChina_getQR::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params UNUSED_PARAM)
{
	sInput data;
	if (!rhea::json::parse(params, ajaxReq_AliChina_getQR_jsonTrapFunction, &data))
		return;

	u8 urlToBeEmbeddedInQr[256];
	const u64 timeToExitMSec = rhea::getTimeNowMSec() + 10000;
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		if (server->module_alipayChina_askQR (data.name, data.selNum, data.price, false, urlToBeEmbeddedInQr, sizeof(urlToBeEmbeddedInQr)))
		{
			server->sendAjaxAnwer (hClient, ajaxRequestID, urlToBeEmbeddedInQr, rhea::string::utf8::lengthInBytes(urlToBeEmbeddedInQr) + 1);
			return;
		}
		else
		{
			rhea::thread::sleepMSec(1000);
		}
	}

	//richiesta fallita
	urlToBeEmbeddedInQr[0] = 'K';
	urlToBeEmbeddedInQr[1] = 'O';
	urlToBeEmbeddedInQr[2] = 0x00;
	server->sendAjaxAnwer (hClient, ajaxRequestID, urlToBeEmbeddedInQr, 3);

}