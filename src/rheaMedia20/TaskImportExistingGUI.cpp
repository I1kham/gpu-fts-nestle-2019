#include "TaskImportExistingGUI.h"




//*********************************************************************
void TaskImportExistingGUI::run (socketbridge::TaskStatus *status, const char *params)
{
	char templateName[128];
	char templateVer[8];
	char templateSrcPath[512];
	char userGUISrcPath[512];
	char dstPath[512];


	memset(templateName, 0, sizeof(templateName));
	memset(templateVer, 0, sizeof(templateVer));
	memset(templateSrcPath, 0, sizeof(templateSrcPath));
	memset(userGUISrcPath, 0, sizeof(userGUISrcPath));
	memset(dstPath, 0, sizeof(dstPath));

	rhea::string::parser::Iter iter;
	rhea::string::parser::Iter iter2;
	const char SEP = '§';
	iter.setup(params);

	while (1)
	{
		if (!rhea::string::parser::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyCurStr(templateName, sizeof(templateName));

		if (!rhea::string::parser::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyCurStr(templateVer, sizeof(templateVer));
			
		if (!rhea::string::parser::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyCurStr(templateSrcPath, sizeof(templateSrcPath));
		rhea::fs::sanitizePathInPlace(templateSrcPath);

		if (!rhea::string::parser::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyCurStr(userGUISrcPath, sizeof(userGUISrcPath));
		rhea::fs::sanitizePathInPlace(userGUISrcPath);


		if (!rhea::string::parser::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyCurStr(dstPath, sizeof(dstPath));
		rhea::fs::sanitizePathInPlace(dstPath);


		//tutti i parametri sono validi


		//creo il folder dstPath
		if (!rhea::fs::folderCreate(dstPath))
		{
			status->setMessage("Error creating %s", dstPath);
			return;
		}

		//copio il template dentro dstPath
		status->setMessage("Copying template files...");
		if (!rhea::fs::folderCopy(templateSrcPath, dstPath))
		{
			status->setMessage("Error copying files");
			return;
		}

		//copio le opzioni utente
		char src[512];
		char dst[512];
		status->setMessage("Copying existing gui files...");

		sprintf_s(src, sizeof(src), "%s/web/upload", userGUISrcPath);
		sprintf_s(dst, sizeof(dst), "%s/web/upload", dstPath);
		if (!rhea::fs::folderCopy(src, dst))
		{
			status->setMessage("Error copying files 1");
			return;
		}

		sprintf_s(src, sizeof(src), "%s/web/config", userGUISrcPath);
		sprintf_s(dst, sizeof(dst), "%s/web/config", dstPath);
		if (!rhea::fs::folderCopy(src, dst))
		{
			status->setMessage("Error copying files 2");
			return;
		}

		sprintf_s(src, sizeof(src), "%s/web/backoffice/guidb.db3", userGUISrcPath);
		sprintf_s(dst, sizeof(dst), "%s/web/backoffice/guidb.db3", dstPath);
		if (!rhea::fs::fileCopy(src, dst))
		{
			status->setMessage("Error copying files 2");
			return;
		}

		status->setMessage("OK");
		return;
	}

	//se arriviamo qui, c'e' stato un errore coi parametri
	status->setMessage("Invalid parameters");
}