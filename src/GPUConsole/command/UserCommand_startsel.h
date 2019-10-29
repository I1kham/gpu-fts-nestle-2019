#ifndef _UserCommand_startsel_h_
#define _UserCommand_startsel_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_startsel
 *
 */
class UserCommand_startsel : public UserCommand
{
public:
					UserCommand_startsel() : UserCommand ("startsel") { }

	const char*		getExplain() const;

	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const;

};




#endif // _UserCommand_startsel_h_
