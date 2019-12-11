#include "CmdHandler_ajaxReqTaskSpawn.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	char name[128];
	char params[1024];


	sInput()					{ name[0] = params[0] = 0; }
};

//***********************************************************
bool ajaxReqTaskSpawn_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "name") == 0)
	{
		strcpy_s(input->name, sizeof(input->name), fieldValue);
		return true;
	}
	else if (strcasecmp(fieldName, "params") == 0)
	{
		if (fieldValue[0] != 0x00)
			strcpy_s(input->params, sizeof(input->params), fieldValue);
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReqTaskSpawn::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params)
{
	sInput data;
	if (!rhea::json::parse(params, ajaxReqTaskSpawn_jsonTrapFunction, &data))
		return;

	char resp[16];
	u32 uid = 0;
	server->taskSpawnAndRun(data.name, data.params, &uid);
	sprintf_s(resp, sizeof(resp), "%d", uid);
	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));
}
