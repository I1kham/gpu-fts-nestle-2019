#ifndef _ISimpleLogger_h_
#define _ISimpleLogger_h_
#include "rheaDataTypes.h"

namespace rhea
{
    /******************************************
     * ISimpleLogger
     *
     */
    class ISimpleLogger
    {
    public:
                            ISimpleLogger()                                     { }
        virtual             ~ISimpleLogger()                                    { }

        virtual void        incIndent()  = 0;
        virtual void        decIndent() = 0;
        virtual void        log (const char *format, ...) = 0;
    };
} //namespace rhea

#endif // _IProtocol_h_

