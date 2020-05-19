#include "CmdHandler_ajaxReqIsManualInstalled.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


//***********************************************************
void CmdHandler_ajaxReqIsManualInstalled::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params UNUSED_PARAM)
{
	u8 pathToManualFolder[256];
	sprintf_s((char*)pathToManualFolder, sizeof(pathToManualFolder), "%s/last_installed/manual", rhea::getPhysicalPathToAppFolder());

	u8 manualFolderName[128];
	manualFolderName[0] = 0;

	OSFileFind h;
	if (rhea::fs::findFirst(&h, pathToManualFolder, (const u8*)"*.*"))
	{
		do
		{
			if (!rhea::fs::findIsDirectory(h))
				continue;
			const u8 *folderName = rhea::fs::findGetFileName(h);
			if (folderName[0] == '.')
				continue;
			sprintf_s((char*)manualFolderName, sizeof(manualFolderName), "%s", folderName);
			break;

		} while (rhea::fs::findNext(h));
		rhea::fs::findClose(h);
	}

	if (manualFolderName[0] == 0x00)
	{
		manualFolderName[0] = 'K';
		manualFolderName[1] = 'O';
		manualFolderName[2] = 0x00;
	}

	server->sendAjaxAnwer(hClient, ajaxRequestID, manualFolderName, (u16)rhea::string::utf8::lengthInBytes(manualFolderName));
}
