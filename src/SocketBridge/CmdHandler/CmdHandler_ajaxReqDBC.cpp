#include "CmdHandler_ajaxReqDBC.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	char path[256];
};

//***********************************************************
bool ajaxReqDBC_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "path") == 0)
	{
		strncpy (input->path, fieldValue, sizeof(input->path)-1);
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReqDBC::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params)
{
	u16 handle = 0;
	sInput data;
	if (rhea::json::parse(params, ajaxReqDBC_jsonTrapFunction, &data))
		handle = server->DB_getOrCreateHandle(data.path);

	char resp[64];
	sprintf(resp, "{\"handle\":%d}", handle);
	server->sendAjaxAnwer (hClient, ajaxRequestID, resp, (u16)strlen(resp));
}