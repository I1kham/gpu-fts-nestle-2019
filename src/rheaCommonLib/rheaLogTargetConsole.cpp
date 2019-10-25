#include "OS/OS.h"
#include "rheaLogTargetConsole.h"


using namespace rhea;

//**********************************************************
void LogTargetConsole::doLog (u32 channel UNUSED_PARAM, const char *msg)
{
    printf ("LOG: %s\n", msg);
}

