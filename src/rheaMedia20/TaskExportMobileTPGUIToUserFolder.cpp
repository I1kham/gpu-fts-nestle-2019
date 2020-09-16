#include "TaskExportMobileTPGUIToUserFolder.h"
#include "../rheaCommonLib/compress/rheaCompress.h"



//*********************************************************************
void TaskExportMobileTPGUIToUserFolder::run(socketbridge::TaskStatus *status, const u8 *params)
{
	u8 srcTempFolderName[64];
	u8 dstPath[512];
	u8 dstPathTemp[512];

	memset(srcTempFolderName, 0, sizeof(srcTempFolderName));
	memset(dstPath, 0, sizeof(dstPath));
	memset(dstPathTemp, 0, sizeof(dstPathTemp));

	rhea::string::utf8::Iter iter;
	rhea::string::utf8::Iter iter2;
	const rhea::UTF8Char SEP("§");
	iter.setup(params);

	while (1)
	{
		//folder src
		if (!rhea::string::utf8::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyAllStr(srcTempFolderName, sizeof(srcTempFolderName));
		rhea::fs::sanitizePathInPlace(srcTempFolderName);

		//path dst
		iter.advanceOneChar();
		if (!rhea::string::utf8::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyAllStr(dstPath, sizeof(dstPath));
		rhea::fs::sanitizePathInPlace(dstPath);

		//path dst/temp
		sprintf_s((char*)dstPathTemp, sizeof(dstPathTemp), "%s/temp", dstPath);


		//copia la cartella src in dst/temp, skippando alcune sottocartelle
		u8 src[512];
		{
			u8 folderToSkip1[256];
			sprintf_s((char*)folderToSkip1, sizeof(folderToSkip1), "%s/temp/%s/web/backoffice", rhea::getPhysicalPathToAppFolder(), srcTempFolderName);

			u8 folderToSkip2[256];
			sprintf_s((char*)folderToSkip2, sizeof(folderToSkip2), "%s/temp/%s/web/js/dev", rhea::getPhysicalPathToAppFolder(), srcTempFolderName);

			u8 *folderToSkip[3] = { folderToSkip1 , folderToSkip2, NULL };
			sprintf_s((char*)src, sizeof(src), "%s/temp/%s", rhea::getPhysicalPathToAppFolder(), srcTempFolderName);
			rhea::fs::folderCopy(src, dstPathTemp, folderToSkip);
		}

		//Creo la cartella dst/temp/backoffice
		sprintf_s((char*)src, sizeof(src), "%s/web/backoffice", dstPathTemp);
		rhea::fs::folderCreate(src);

		//ci copio dentro il db
		{
			u8 dstBackoffice[256];
			sprintf_s((char*)dstBackoffice, sizeof(dstBackoffice), "%s/web/backoffice/guidb.db3", dstPathTemp);
			sprintf_s((char*)src, sizeof(src), "%s/temp/%s/web/backoffice/guidb.db3", rhea::getPhysicalPathToAppFolder(), srcTempFolderName);
			rhea::fs::fileCopy(src, dstBackoffice);
		}


		//a questo punto, in dst/temp ho tutta la GUI.
		//Devo zippare tutto il contenuto di dst/temp/web e poi eliminare l'intera cartella temp
		rhea::CompressUtility cu;
		sprintf_s((char*)src, sizeof(src), "%s/mobile.rheaRasGuiTP", dstPath);
		cu.begin (src, 8);
			sprintf_s ((char*)src, sizeof(src), "%s/web", dstPathTemp);
			cu.addFilesInFolder (src, (const u8*)"", true);
		cu.end();


		//elimino dst/temp
		rhea::fs::deleteAllFileInFolderRecursively (dstPathTemp, true);

		//fine
		status->setMessage("OK");
		return;
	}

	//se arriviamo qui, c'e' stato un errore coi parametri
	status->setMessage("Invalid parameters");
}