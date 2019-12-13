#include "TaskExportGUIToUserFolder.h"




//*********************************************************************
void TaskExportGUIToUserFolder::run(socketbridge::TaskStatus *status, const char *params)
{
	char srcTempFolderName[64];
	char dstPath[512];

	memset(srcTempFolderName, 0, sizeof(srcTempFolderName));
	memset(dstPath, 0, sizeof(dstPath));

	rhea::string::parser::Iter iter;
	rhea::string::parser::Iter iter2;
	const char SEP = '§';
	iter.setup(params);

	while (1)
	{
		if (!rhea::string::parser::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyCurStr(srcTempFolderName, sizeof(srcTempFolderName));
		rhea::fs::sanitizePathInPlace(srcTempFolderName);

		if (!rhea::string::parser::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyCurStr(dstPath, sizeof(dstPath));
		rhea::fs::sanitizePathInPlace(dstPath);

		char src[512];
		{
			char folderToSkip1[256];
			sprintf_s(folderToSkip1, sizeof(folderToSkip1), "%s/temp/%s/web/backoffice", rhea::getPhysicalPathToAppFolder(), srcTempFolderName);

			char folderToSkip2[256];
			sprintf_s(folderToSkip2, sizeof(folderToSkip2), "%s/temp/%s/web/js/dev", rhea::getPhysicalPathToAppFolder(), srcTempFolderName);

			char *folderToSkip[4] = { folderToSkip1 , folderToSkip2, NULL, NULL };

			sprintf_s(src, sizeof(src), "%s/temp/%s", rhea::getPhysicalPathToAppFolder(), srcTempFolderName);
			rhea::fs::folderCopy(src, dstPath, folderToSkip);
		}

		//Creo la cartella dst/backoffice
		sprintf_s(src, sizeof(src), "%s/web/backoffice", dstPath);
		rhea::fs::folderCreate(src);

		//ci copio dentro il db
		{
			char dstBackoffice[256];
			sprintf_s(dstBackoffice, sizeof(dstBackoffice), "%s/web/backoffice/guidb.db3", dstPath);
			sprintf_s(src, sizeof(src), "%s/temp/%s/web/backoffice/guidb.db3", rhea::getPhysicalPathToAppFolder(), srcTempFolderName);
			rhea::fs::fileCopy(src, dstBackoffice);
		}

		status->setMessage("OK");
		return;
	}

	//se arriviamo qui, c'e' stato un errore coi parametri
	status->setMessage("Invalid parameters");
}