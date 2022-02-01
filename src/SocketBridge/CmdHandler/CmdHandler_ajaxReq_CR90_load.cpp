#include "CmdHandler_ajaxReq_CR90_load.h"
#include "../SocketBridge.h"
#include "../CR90File.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_CR90_load::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params UNUSED_PARAM)
{
	rhea::Allocator *allocator = rhea::getSysHeapAllocator();
	
	CR90File cr90;
	cr90.load();

	u32 sizeOfResult = cr90.getNumCellX() * cr90.getNumCellY() * 8;
	u8 *result = RHEAALLOCT (u8*, allocator, sizeOfResult);
	memset (result, 0, sizeOfResult);

	u32 index = 0;
	for (u16 y = 0; y < cr90.getNumCellY(); y++)
	{
		for (u16 x = 0; x < cr90.getNumCellX(); x++)
		{
			rhea::string::utf8::appendU16 (result, sizeOfResult, cr90.getValue(index));
			rhea::string::utf8::appendUTF8Char (result, sizeOfResult, ",");
			index++;
		}
	}
	rhea::string::utf8::appendUTF8Char (result, sizeOfResult, "-1");

	server->sendAjaxAnwer(hClient, ajaxRequestID, result, rhea::string::utf8::lengthInBytes(result));
	RHEAFREE(allocator, result);
}

