#ifndef _UserCommand_setposmacina_h_
#define _UserCommand_setposmacina_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_setposmacina
 *
 */
class UserCommand_setposmacina : public UserCommand
{
public:
					UserCommand_setposmacina() : UserCommand ("setposmacina") { }

	const char*		getExplain() const;

	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const;

};




#endif // _UserCommand_setposmacina_h_
