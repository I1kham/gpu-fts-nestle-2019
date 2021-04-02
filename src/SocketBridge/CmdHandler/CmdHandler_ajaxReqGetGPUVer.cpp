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
		u8 answer[4] = { 'K', 'O', 0x00, 0x00 };
		server->sendAjaxAnwer(hClient, ajaxRequestID, answer, 3);
	}

	/*
	u8		fileName[256];
	u8		*pGpuVersion = NULL;
	u32		fileSize;
	sprintf_s((char*)fileName, sizeof(fileName), "%s/current/gpu/ver.txt", rhea::getPhysicalPathToAppFolder());
	rhea::fs::sanitizePathInPlace(fileName, strlen((char *)fileName));

	FILE *fd = rhea::fs::fileOpenForReadText(fileName);
	if(NULL != fd && 0 != (fileSize = (u32)rhea::fs::filesize(fd)))
	{
		pGpuVersion = new u8[fileSize + 1];
		
		memset(pGpuVersion, 0, fileSize + 1);
		fread(pGpuVersion, sizeof(u8), fileSize, fd);
		fclose(fd);
	}
	
	if(NULL == pGpuVersion)
	{
		pGpuVersion = new u8[3];
		pGpuVersion[0] = 'K';
		pGpuVersion[1] = 'O';
		pGpuVersion[2] = 0x00;
	}
	
	server->sendAjaxAnwer(hClient, ajaxRequestID, pGpuVersion, (u16)rhea::string::utf8::lengthInBytes(pGpuVersion));
	
	delete [] pGpuVersion;
	*/
}
