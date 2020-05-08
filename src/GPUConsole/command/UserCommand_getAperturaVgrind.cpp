#include "UserCommand_getAperturaVgrind.h"

//**********************************************************
const char*	UserCommand_getAperturaVgrind::getExplain() const
{ 
	return "getaperturavgrind [macina_1o2]\nse non specificata, si assume macina 1";
}

//**********************************************************
void UserCommand_getAperturaVgrind::handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
{
	rhea::string::parser::Iter src, temp;
	src.setup(command);

	i32 macina_1o2 = 1;
	while (1)
	{
		if (!rhea::string::parser::advanceUntil(src, " ", 1))
			break;
		rhea::string::parser::toNextValidChar(src);

		if (!rhea::string::parser::extractInteger(src, &macina_1o2))
		{
			log->log("syntax error, expecting integer parameter [macina_1o2]\n");
			return;
		}

		if (macina_1o2 != 1 && macina_1o2 != 2)
		{
			log->log("invalid parameter [macina_1o2], must be ==1 or ==2\n");
			return;
		}
		break;
	}


	rhea::app::GetAperturaVGrind::ask (ch, proto, macina_1o2);
	log->log("request sent\n");

}