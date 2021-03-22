#include "CmdHandler_ajaxReqGetGPUVer.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqGetGPUVer::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params UNUSED_PARAM)
{
	u8		fileName[256], *pGpuVersion = NULL;
	FILE	*fd;
	u64		fileSize;
	
	sprintf_s((char*)fileName, sizeof(fileName), "%s/current/gpu/ver.txt", rhea::getPhysicalPathToAppFolder());
	rhea::fs::sanitizePathInPlace(fileName, strlen((char *)fileName));

	fd = rhea::fs::fileOpenForReadText(fileName);
	if(NULL != fd && 0 != (fileSize = rhea::fs::filesize(fd)))
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
}
