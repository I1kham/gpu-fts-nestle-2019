#include "CmdHandler_ajaxReqGetLastInstalledCPUFilename.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqGetLastInstalledCPUFilename::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params UNUSED_PARAM)
{
	u8			fullPath[256], fileName[128];
	u32			strLen;
	OSFileFind	h;

	strLen = sprintf_s((char*)fullPath, sizeof(fullPath), "%s/last_installed/cpu", rhea::getPhysicalPathToAppFolder());
	rhea::fs::sanitizePathInPlace(fullPath, strLen);

	memset(fileName, 0, sizeof fileName);

	if (rhea::fs::findFirst(&h, fullPath, (const u8*)"*.mhx"))
	{
		bool	exitLoop = false;

		do
		{
			if (!rhea::fs::findIsDirectory(h))
			{
				const u8* folderName = rhea::fs::findGetFileName(h);
				if (folderName[0] != '.')
				{
					sprintf_s((char*)fileName, sizeof fileName, "%s", folderName);
					exitLoop = true;
				}
			}
		} while (false == exitLoop && rhea::fs::findNext(h));
		rhea::fs::findClose(h);
	}

	if (fileName[0] == 0x00)
	{
		fileName[0] = 'K';
		fileName[1] = 'O';
	}

	server->sendAjaxAnwer(hClient, ajaxRequestID, fileName, (u16)rhea::string::utf8::lengthInBytes(fileName));
}
