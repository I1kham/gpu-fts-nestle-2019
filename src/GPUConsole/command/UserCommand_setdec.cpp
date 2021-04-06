#include "UserCommand_setdec.h"

//**********************************************************
const char*	UserCommand_setdec::getExplain() const
{ 
	return "setdec [which] [value]\nwhich >= 1 && <=10 per i decounter prodotto da 1 a 10\nwhich==11 per il water filter\nwhich==12 per coffee brewer\nwhich==13 per il coffee ground";
}

//**********************************************************
void UserCommand_setdec::handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
{
	rhea::string::utf8::Iter src, temp;
	src.setup((const u8*)command);

	const rhea::UTF8Char cSpace(" ");
	if (!rhea::string::utf8::advanceUntil(src, &cSpace, 1))
	{
		log->log("syntax error, expecting parameter [which]\n");
		return;
	}
	

	i32 which = 0;
	rhea::string::utf8::toNextValidChar(src);
	if (!rhea::string::utf8::extractInteger(src, &which))
	{
		log->log("syntax error, expecting integer parameter [which]\n");
		return;
	}


	i32 value = 0;
	rhea::string::utf8::toNextValidChar(src);
	if (!rhea::string::utf8::extractInteger(src, &value))
	{
		log->log("syntax error, expecting integer parameter [value]\n");
		return;
	}

	if (which >= 1 && which <= 13)
	{
		if (value >= 0 && value <= 0xFFFF)
		{
			rhea::app::SetDecounter::ask(ch, proto, (cpubridge::eCPUProg_decounter)which, (u16)value);
			log->log("request sent\n");
		}
		else
			log->log("syntax error, invalid [value].\n");
	}
	else
		log->log("syntax error, invalid [which].\n");
}