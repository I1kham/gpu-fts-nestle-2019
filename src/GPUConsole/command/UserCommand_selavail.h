#ifndef _UserCommand_selavail_h_
#define _UserCommand_selavail_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_cls
 *
 */
class UserCommand_selavail : public UserCommand
{
public:
				UserCommand_selavail() : UserCommand ("selavail") { }

	void		handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
				{
					rhea::app::CurrentSelectionAvailability::ask(ch, proto);
				}

	
};




#endif // _UserCommand_selavail_h_
