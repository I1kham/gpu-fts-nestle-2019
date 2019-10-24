#ifndef _UserCommand_cpumsg_h_
#define _UserCommand_cpumsg_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_cls
 *
 */
class UserCommand_cpumsg : public UserCommand
{
public:
				UserCommand_cpumsg() : UserCommand ("cpumsg") { }

	void		handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
				{
					rhea::app::CurrentCPUMessage::ask(ch, proto);
				}

	
};




#endif // _UserCommand_cpumsg_h_
