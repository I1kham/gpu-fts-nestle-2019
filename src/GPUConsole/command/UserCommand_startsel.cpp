#include "UserCommand_startsel.h"

//**********************************************************
const char*	UserCommand_startsel::getExplain() const
{ 
	return "startsel [selnum]\nselum >= 1 && selnnum <=48";
}

//**********************************************************
void UserCommand_startsel::handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
{
	rhea::string::utf8::Iter src, temp;
	src.setup((const u8*)command);

	const rhea::UTF8Char cSpace(" ");
	if (!rhea::string::utf8::advanceUntil(src, &cSpace, 1))
	{
		log->log("syntax error, expecting parameter [selnum]\n");
		return;
	}
	rhea::string::utf8::toNextValidChar(src);

	i32 selNum = 0;
	if (!rhea::string::utf8::extractInteger(src, &selNum))
	{
		log->log("syntax error, expecting integer parameter [selnum]\n");
		return;
	}

	if (selNum >= 1 && selNum <= 48)
	{
		rhea::app::ExecuteSelection::ask(ch, proto, (u8)selNum);
		log->log("request sent\n");
	}
	else
		log->log("syntax error, invalid [selnum]. Good values are between 1 and 48\n");
}