#include "CmdHandler_ajaxReqDBQ.h"
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
bool ajaxReqDBQ_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
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
		strncpy(input->sql, fieldValue, sizeof(input->sql) - 1);
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReqDBQ::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params)
{
	const rhea::SQLRst *rst = NULL;
	sInput data;

    if (rhea::json::parse(params, ajaxReqDBQ_jsonTrapFunction, &data))
		rst = server->DB_q(data.dbHandle, data.sql);
	

	if (NULL == rst)
	{
		char resp[64];
		sprintf(resp, "{\"data\":\"KO\"}");
		server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));
	}
	else
	{
		rhea::Allocator *allocator = rhea::memory_getScrapAllocator();


		const u32 sizeOfBuffer = rst->blob_calcMemoryNeeded();
		u8 *buffer = (u8*)RHEAALLOC(allocator, sizeOfBuffer);
		const u32 nBytesInBlob = rst->blob_copyToString(buffer, sizeOfBuffer);
		
		server->sendAjaxAnwer (hClient, ajaxRequestID, (const char*)buffer, nBytesInBlob);

		RHEAFREE(allocator, buffer);
	}
}
