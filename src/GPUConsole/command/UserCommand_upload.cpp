#include "UserCommand_upload.h"

//**********************************************************
const char*	UserCommand_upload::getExplain() const
{ 
	return "upload [filename]\nupload the [filename] to the SMU.\nThe SMU will store the file in the temp folder";
}

//**********************************************************
void UserCommand_upload::handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
{
	rhea::string::parser::Iter src, temp;
	src.setup(command);

	if (!rhea::string::parser::advanceUntil(src, " ", 1))
	{
		log->log("syntax error, expecting parameter [filename]\n");
		return;
	}
	rhea::string::parser::toNextValidChar(src);

	if (!rhea::string::parser::extractValue(src, &temp, " ", 1))
	{
		log->log("syntax error, expecting parameter [filename]\n");
		return;
	}

	char param1[512];
	temp.copyCurStr(param1, sizeof(param1));

	char filenameOnly[128];
	rhea::fs::extractFileNameWithExt(param1, filenameOnly, sizeof(filenameOnly));

	char uploadCommand[128];
	sprintf_s(uploadCommand, sizeof(uploadCommand), "store:%s", filenameOnly);



	rhea::app::FileTransfer::Handle handle;
	log->incIndent();

	if (ftransf->startFileUpload(ch, proto, rhea::getTimeNowMSec(), param1, uploadCommand, &handle))
		log->log("file transfer started. Handle [0x%08X]\n", handle.asU32());
	else
		log->log("file transfer FAILED to start\n");
	
	log->decIndent();
}