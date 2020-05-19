#include "UserCommand_cpuprogcmd.h"

//**********************************************************
const char*	UserCommand_cpuprogcmd::getExplain() const
{ 
	return	"cpuprogcmd [cmd] | [p1] | [p2] | [p3] | [p4]\n"
			"[cmd]:1=enterprog, 2=cleaning...\n"
			"[p1..4]=optional, depending on [cmd]\n";
}

//**********************************************************
void UserCommand_cpuprogcmd::handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
{
	rhea::string::utf8::Iter src, temp;
	src.setup((const u8*)command);

	const rhea::UTF8Char cSpace(" ");
	if (!rhea::string::utf8::advanceUntil(src, &cSpace, 1))
	{
		log->log("syntax error, expecting parameter [cmd]\n");
		return;
	}
	rhea::string::utf8::toNextValidChar(src);

	i32 cmd = 0;
	if (!rhea::string::utf8::extractInteger(src, &cmd))
	{
		log->log("syntax error, expecting integer parameter [cmd]\n");
		return;
	}


	//param1..4 sono opzionali
	u8 params[4] = { 0,0,0,0 };
	u8 ct = 0;
	while (ct < 4)
	{
		rhea::string::utf8::toNextValidChar(src);
		i32 par;
		if (!rhea::string::utf8::extractInteger(src, &par))
			break;
		params[ct++] = (u8)par;
	}

	rhea::app::ExecuteProgramCmd::ask(ch, proto, (cpubridge::eCPUProgrammingCommand)cmd, params[0], params[1], params[2], params[3]);
	log->log("request sent\n");

}