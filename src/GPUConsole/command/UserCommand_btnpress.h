#ifndef _UserCommand_btnpress_h_
#define _UserCommand_btnpress_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_startsel
 *
 */
class UserCommand_btnpress : public UserCommand
{
public:
					UserCommand_btnpress() : UserCommand ("btnpress") { }

	const char*		getExplain() const;

	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const;

};




#endif // _UserCommand_btnpress_h_
