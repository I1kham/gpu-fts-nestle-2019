#ifndef _Terminal_h_
#define _Terminal_h_
#include "winTerminal.h"
#include "enumAndDefine.h"


/***************************************************
 * Terminal
 *
 */
class Terminal: public WinTerminal
{
public:
					Terminal (HThreadMsgW msgQHandleW);
					~Terminal();


	void			virt_onUserCommand(const char *s);

private:
	HThreadMsgW		msgQHandleW;
};




#endif // _Terminal_h_
