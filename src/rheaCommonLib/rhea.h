#ifndef _rhea_h_
#define _rhea_h_
#include "OS/OS.h"
#include "rheaEnumAndDefine.h"
#include "rheaMemory.h"
#include "rheaLogger.h"


namespace rhea
{
    bool                init (void *platformSpecificInitData);
    void                deinit();

    extern Logger       *logger;

    inline const char*  getPhysicalPathToAppFolder()					{ return OS_getAppPathNoSlash();}

    inline void         shell_runCommandNoWait(const char *cmd)          { OS_runShellCommandNoWait(cmd); }


} //namespace rhea


#endif // _rhea_h_

