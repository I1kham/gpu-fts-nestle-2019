#ifndef _UserCommand_cpuiniparam_h_
#define _UserCommand_cpuiniparam_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_cls
 *
 */
class UserCommand_cpuiniparam : public UserCommand
{
public:
				UserCommand_cpuiniparam() : UserCommand ("cpuiniparam") { }

	void		handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
				{
					rhea::app::CurrentCPUInitParam::ask(ch, proto);
				}

	
};




#endif // _UserCommand_cpuiniparam_h_
