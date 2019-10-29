#ifndef _UserCommand_startcleaning_h_
#define _UserCommand_startcleaning_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_startcleaning
 *
 */
class UserCommand_startcleaning : public UserCommand
{
public:
					UserCommand_startcleaning() : UserCommand ("startcleaning") { }

	const char*		getExplain() const;

	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const;

};




#endif // _UserCommand_startcleaning_h_
