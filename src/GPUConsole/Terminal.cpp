#include "Terminal.h"


//*******************************************************
Terminal::Terminal (HThreadMsgW msgQHandleW_IN)
{
	msgQHandleW = msgQHandleW_IN;
}

//*******************************************************
Terminal::~Terminal()
{
}

//*******************************************************
void Terminal::virt_onUserCommand(const char *s)
{
	if (strcasecmp(s, "exit") == 0)
		this->exitLoop();
	else if (strcasecmp(s, "cls") == 0)
		this->cls();
	else
		rhea::thread::pushMsg(msgQHandleW, MSGQ_USER_INPUT, (u32)0, s, 1 + strlen(s));
}