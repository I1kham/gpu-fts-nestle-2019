#include "CmdHandler_ajaxReqFSRheaUnzip.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../..//rheaCommonLib/compress/rheaCompress.h"


using namespace socketbridge;

struct sInput
{
	u8 src[512];
	u8 dst[512];
	u8 mkdir;
};

//***********************************************************
bool CmdHandler_ajaxReqFSRheaUnzip_jsonTrapFunction(const u8* fieldName, const u8* fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (rhea::string::utf8::areEqual (fieldName, (const u8*)"src", true))
	{
		rhea::fs::sanitizePath(fieldValue, input->src, sizeof(input->src) - 1);
		return true;
	}
	else if (rhea::string::utf8::areEqual (fieldName, (const u8*)"dst", true))
	{
		rhea::fs::sanitizePath(fieldValue, input->dst, sizeof(input->dst) - 1);
		return true;
	}
	else if (rhea::string::utf8::areEqual (fieldName, (const u8*)"mkdir", true))
	{
		input->mkdir = (u8)rhea::string::utf8::toI32(fieldValue);
		if (input->mkdir != 1)
			input->mkdir = 0;
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReqFSRheaUnzip::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params)
{
	sInput data;
	if (!rhea::json::parse(params, CmdHandler_ajaxReqFSRheaUnzip_jsonTrapFunction, &data))
		return;

	char resp[4];
	sprintf(resp, "KO");
	if (rhea::fs::fileExists(data.src))
	{
		if (data.mkdir == 1)
			rhea::fs::folderCreate(data.dst);

		if (rhea::fs::folderExists(data.dst))
		{
			if (rhea::CompressUtility::decompresAll (data.src, data.dst))
				sprintf(resp, "OK");
		}
	}		
	
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
