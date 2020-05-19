#ifndef _rheaLogTargetFile_h_
#define _rheaLogTargetFile_h_
#include "rheaLogger.h"


namespace rhea
{
    /*===============================================================
     *
     *=============================================================*/
    class LogTargetFile : public ILogTarget
    {
    public:
                        LogTargetFile ()														{ filename=NULL; }
                        ~LogTargetFile();

        bool			init (const char *strNomeFileOut, bool bDeleteFileOnStartup=true);

                        //===============================================
        void			doLog	(u32 channel, const char *msg);

    private:
        u8			    *filename;
    };
} //namespace rhea

#endif // _rheaLogTargetFile_h_
