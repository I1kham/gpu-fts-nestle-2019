#ifndef _UserCommand_setAperturaVgrind_h_
#define _UserCommand_setAperturaVgrind_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_setAperturaVgrind
 *
 */
class UserCommand_setAperturaVgrind : public UserCommand
{
public:
					UserCommand_setAperturaVgrind() : UserCommand ("setaperturavgrind") { }

	const char*		getExplain() const;

	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const;

};




#endif // _UserCommand_setAperturaVgrind_h_
