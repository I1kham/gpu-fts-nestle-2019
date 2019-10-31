#include "UserCommand_download.h"

//**********************************************************
const char*	UserCommand_download::getExplain() const
{ 
	return "download [what]\n[what] = audit[n] | da3[n] | test"; 
}

//**********************************************************
void UserCommand_download::handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
{
	rhea::string::parser::Iter src, temp;
	src.setup(command);

	if (!rhea::string::parser::advanceUntil(src, " ", 1))
	{
		log->log("syntax error, expecting parameter [what]\n");
		return;
	}
	rhea::string::parser::toNextValidChar(src);

	if (!rhea::string::parser::extractValue(src, &temp, " ", 1))
	{
		log->log("syntax error, expecting parameter [what]\n");
		return;
	}

	char what[64];
	temp.copyCurStr(what, sizeof(what));


	rhea::app::FileTransfer::Handle handle;
	log->incIndent();


	char downloadedFilePathAndName[512];
	downloadedFilePathAndName[0] = 0x00;
	if (strcasecmp(what, "test") == 0)
	{
		sprintf_s(downloadedFilePathAndName, sizeof(downloadedFilePathAndName), "%s/file_downloadata_da_smu", rhea::getPhysicalPathToWritableFolder());
	}
	else if (strncasecmp(what, "audit", 5) == 0)
	{
		sprintf_s(downloadedFilePathAndName, sizeof(downloadedFilePathAndName), "%s/audit.txt", rhea::getPhysicalPathToWritableFolder());
	}
	else if (strncasecmp(what, "da3", 3) == 0)
	{
		sprintf_s(downloadedFilePathAndName, sizeof(downloadedFilePathAndName), "%s/vmcDataFile.da3", rhea::getPhysicalPathToWritableFolder());
	}
	else
	{
		log->log("syntax error, invalid [what]\n");
	}




	if (downloadedFilePathAndName[0] != 0x00)
	{
		log->outText(false, true, true, "dst file is: %s\n", downloadedFilePathAndName);
		if (ftransf->startFileDownload(ch, proto, rhea::getTimeNowMSec(), what, downloadedFilePathAndName, &handle))
			log->log("file transfer started. Handle [0x%08X]\n", handle.asU32());
		else
			log->log("file transfer FAILED to start\n");
	}
	
	log->decIndent();
}