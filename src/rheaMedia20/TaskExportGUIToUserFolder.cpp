#include "TaskExportGUIToUserFolder.h"




//*********************************************************************
void TaskExportGUIToUserFolder::run(socketbridge::TaskStatus *status, const u8 *params)
{
	u8 srcTempFolderName[64];
	u8 dstPath[512];

	memset(srcTempFolderName, 0, sizeof(srcTempFolderName));
	memset(dstPath, 0, sizeof(dstPath));

	rhea::string::utf8::Iter iter;
	rhea::string::utf8::Iter iter2;
	const rhea::UTF8Char SEP("§");
	iter.setup(params);

	while (1)
	{
		if (!rhea::string::utf8::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyAllStr(srcTempFolderName, sizeof(srcTempFolderName));
		rhea::fs::sanitizePathInPlace(srcTempFolderName);

		iter.advanceOneChar();
		if (!rhea::string::utf8::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyAllStr(dstPath, sizeof(dstPath));
		rhea::fs::sanitizePathInPlace(dstPath);

		u8 src[512];
		{
			u8 folderToSkip1[256];
			sprintf_s((char*)folderToSkip1, sizeof(folderToSkip1), "%s/temp/%s/web/backoffice", rhea::getPhysicalPathToAppFolder(), srcTempFolderName);

			u8 folderToSkip2[256];
			sprintf_s((char*)folderToSkip2, sizeof(folderToSkip2), "%s/temp/%s/web/js/dev", rhea::getPhysicalPathToAppFolder(), srcTempFolderName);

			u8 *folderToSkip[4] = { folderToSkip1 , folderToSkip2, NULL, NULL };

			sprintf_s((char*)src, sizeof(src), "%s/temp/%s", rhea::getPhysicalPathToAppFolder(), srcTempFolderName);
			rhea::fs::folderCopy(src, dstPath, folderToSkip);
		}

		//Creo la cartella dst/backoffice
		sprintf_s((char*)src, sizeof(src), "%s/web/backoffice", dstPath);
		rhea::fs::folderCreate(src);

		//ci copio dentro il db
		{
			u8 dstBackoffice[256];
			sprintf_s((char*)dstBackoffice, sizeof(dstBackoffice), "%s/web/backoffice/guidb.db3", dstPath);
			sprintf_s((char*)src, sizeof(src), "%s/temp/%s/web/backoffice/guidb.db3", rhea::getPhysicalPathToAppFolder(), srcTempFolderName);
			rhea::fs::fileCopy(src, dstBackoffice);
		}

		status->setMessage("OK");
		return;
	}

	//se arriviamo qui, c'e' stato un errore coi parametri
	status->setMessage("Invalid parameters");
}