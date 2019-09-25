#ifndef _UserCommand_cpustatus_h_
#define _UserCommand_cpustatus_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_cls
 *
 */
class UserCommand_cpustatus : public UserCommand
{
public:
				UserCommand_cpustatus() : UserCommand ("cpustatus") { }

	void		handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
				{
		rhea::app::CurrentCPUStatus::ask(ch, proto);
				}

	
};




#endif // _UserCommand_cpustatus_h_
