#ifndef _UserCommand_getAperturaVgrind_h_
#define _UserCommand_getAperturaVgrind_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_getAperturaVgrind
 *
 */
class UserCommand_getAperturaVgrind : public UserCommand
{
public:
					UserCommand_getAperturaVgrind() : UserCommand ("getaperturavgrind") { }

	const char*		getExplain() const;

	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const;

};




#endif // _UserCommand_getAperturaVgrind_h_
