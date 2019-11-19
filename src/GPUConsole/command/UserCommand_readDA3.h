#ifndef _UserCommand_readDA3_h_
#define _UserCommand_readDA3_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_cls
 *
 */
class UserCommand_readDA3 : public UserCommand
{
public:
				UserCommand_readDA3() : UserCommand ("readDA3") { }

	const char*	getExplain() const
				{
					return "ask CPU to download the DA3 and store it in temp folder\nYou can the download it to your console using the download da3[n] command";
				}

	void		handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
				{
					rhea::app::ReadVMCDataFile::ask(ch, proto);
				}

	
};




#endif // _UserCommand_readDA3_h_
