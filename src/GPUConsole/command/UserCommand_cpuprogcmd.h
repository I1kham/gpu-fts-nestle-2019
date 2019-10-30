#ifndef _UserCommand_cpuprogcmd_h_
#define _UserCommand_cpuprogcmd_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_cpuprogcmd
 *
 */
class UserCommand_cpuprogcmd : public UserCommand
{
public:
					UserCommand_cpuprogcmd() : UserCommand ("cpuprogcmd") { }

	const char*		getExplain() const;

	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const;

};




#endif // _UserCommand_cpuprogcmd_h_
