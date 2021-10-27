#include "CmdHandler_ajaxReqDBE.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	u16		dbHandle;
	u8		sql[4096];
};

//***********************************************************
bool ajaxReqDBE_jsonTrapFunction(const u8 *fieldName, const u8 *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "h") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
		if (h > 0 && h < 0xffff)
			input->dbHandle = (u16)h;
		else
			input->dbHandle = 0;
	}
	else if (strcasecmp((const char*)fieldName, "sql") == 0)
	{
		assert(rhea::string::utf8::lengthInBytes(fieldValue) < sizeof(input->sql));
		rhea::string::utf8::copyStr (input->sql, sizeof(input->sql)-1, fieldValue);
		rhea::string::utf8::decodeURIinPlace(input->sql);
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReqDBE::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params)
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
	
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}