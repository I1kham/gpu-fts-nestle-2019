#include "CmdHandler_ajaxReqGetGPUVer.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqGetGPUVer::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params UNUSED_PARAM)
{
	rhea::Allocator* allocator = rhea::getSysHeapAllocator();

	u8	fileName[256];
	sprintf_s((char*)fileName, sizeof(fileName), "%s/current/gpu/ver.txt", rhea::getPhysicalPathToAppFolder());
	
	u32 fileSize;
	u8* pGpuVersion = rhea::fs::fileCopyInMemory(fileName, allocator, &fileSize);
	if (NULL != pGpuVersion)
	{
		server->sendAjaxAnwer(hClient, ajaxRequestID, pGpuVersion, (u16)fileSize);
		RHEAFREE(allocator, pGpuVersion);
	}
	else
	{
		u8 answer[4] = { '?', '?', '?', 0x00 };
		server->sendAjaxAnwer(hClient, ajaxRequestID, answer, 4);
	}
}
