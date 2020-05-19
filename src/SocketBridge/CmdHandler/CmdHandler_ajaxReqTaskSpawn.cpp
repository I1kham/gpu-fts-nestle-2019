#include "CmdHandler_ajaxReqTaskSpawn.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	u8 name[128];
	u8 params[1024];

	sInput()					{ name[0] = params[0] = 0; }
};

//***********************************************************
bool ajaxReqTaskSpawn_jsonTrapFunction(const u8* const fieldName, const u8* const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "name") == 0)
	{
		rhea::string::utf8::copyStr (input->name, sizeof(input->name), fieldValue);
		return true;
	}
	else if (strcasecmp((const char*)fieldName, "params") == 0)
	{
		if (fieldValue[0] != 0x00)
			rhea::string::utf8::copyStr (input->params, sizeof(input->params), fieldValue);
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReqTaskSpawn::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params)
{
	sInput data;
	if (!rhea::json::parse(params, ajaxReqTaskSpawn_jsonTrapFunction, &data))
		return;

	char resp[16];
	u32 uid = 0;
	server->taskSpawnAndRun((const char*)data.name, data.params, &uid);
	sprintf_s(resp, sizeof(resp), "%d", uid);
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
