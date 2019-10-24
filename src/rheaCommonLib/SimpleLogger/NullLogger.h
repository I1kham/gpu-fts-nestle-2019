#ifndef _rheaNullLogger_h_
#define _rheaNullLogger_h_
#include "ISimpleLogger.h"

namespace rhea
{
    /******************************************
     * NullLogger
     *
     * non stampa niente
     */
    class NullLogger : public ISimpleLogger
    {
    public:
                            NullLogger()                            { }
        virtual             ~NullLogger()                           { }

        void                incIndent()                             { }
        void                decIndent()                             { }
        void                log (const char *format UNUSED_PARAM, ...)           { }
    };
} //namespace rhea
#endif //_rheaNullLogger_h_
