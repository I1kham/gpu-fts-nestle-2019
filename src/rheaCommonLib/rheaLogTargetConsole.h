#ifndef _rheaLogTargetConsole_h_
#define _rheaLogTargetConsole_h_
#include "rheaLogger.h"


namespace rhea
{
    /*===============================================================
     *
     *=============================================================*/
    class LogTargetConsole : public ILogTarget
    {
    public:
                        LogTargetConsole ()														{ }
                        ~LogTargetConsole()                                                     { }

                        //===============================================
        void			doLog	(u32 channel, const char *msg);
    };
} //namespace rhea

#endif // _rheaLogTargetConsole_h_
