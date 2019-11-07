#ifndef _UserCommand_cpuextendedinfo_h_
#define _UserCommand_cpuextendedinfo_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_cls
 *
 */
class UserCommand_cpuextendedinfo : public UserCommand
{
public:
				UserCommand_cpuextendedinfo() : UserCommand ("cpuextendedinfo") { }

	void		handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
				{
					rhea::app::ExtendedConfigInfo::ask(ch, proto);
				}

	
};




#endif // _UserCommand_cpuextendedinfo_h_
