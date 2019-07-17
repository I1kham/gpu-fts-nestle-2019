#ifndef _rheaDateTime_h_
#define _rheaDateTime_h_
#include "rheaDate.h"
#include "rheaTime24.h"

namespace rhea
{
    /**********************************************************
    *	DateTime
    **********************************************************/
    class DateTime
    {
    public:
                DateTime()									{ }
                DateTime(u32 year, u32 month, u32 day)		{ date.setYMD(year, month, day); time.setHMS(0,0,0,0);}
                DateTime(u32 year, u32 month, u32 day,
                        u32 h, u32 m, u32 s, u32 ms=0)		{ date.setYMD(year, month, day); time.setHMS(h,m,s,ms); }

                void	setNow()							{ date.setNow(); time.setNow(); }
        u64		getInternalRappresentation() const			{ u64 d = date.getInternalRappresentation(); u64 t=time.getInternalRappresentation(); return ((d<<32) | t); }
        void	setFromInternalRappresentation(u64 u)		{ date.setFromInternalRappresentation ((u32)((u >> 32) & 0x00000000FFFFFFFF)); time.setFromInternalRappresentation ((u32)(u&0x00000000FFFFFFFF)); }

    public:
                Date	date;
                Time24	time;

                //============== static utils
        static DateTime	Now()								{ DateTime t; t.setNow(); return t; }
    };
} //namespace rhea
#endif // _rheaDateTime_h_
