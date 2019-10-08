#include "CmdHandler_ajaxReqDBE.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	u16		dbHandle;
	char	sql[4096];
};

//***********************************************************
bool ajaxReqDBE_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "h") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
		if (h > 0 && h < 0xffff)
			input->dbHandle = (u16)h;
		else
			input->dbHandle = 0;
	}
	else if (strcasecmp(fieldName, "sql") == 0)
	{
		assert(strlen(fieldValue) < sizeof(input->sql));
		strncpy (input->sql, fieldValue, sizeof(input->sql) - 1);
		rhea::string::convert::decodeURIinPlace(input->sql);
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReqDBE::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params)
{
	sInput data;
	bool bResult = false;

	if (rhea::json::parse(params, ajaxReqDBE_jsonTrapFunction, &data))
		bResult = server->DB_exec (data.dbHandle, data.sql);
	

	char resp[8];
	if (bResult)
		sprintf(resp, "OK");
	else
		sprintf(resp, "KO");
	
	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));
}