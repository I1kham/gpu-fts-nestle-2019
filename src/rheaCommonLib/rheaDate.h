#ifndef _rheaDate_h_
#define _rheaDate_h_
#include "rheaDataTypes.h"

namespace rhea
{
    /**********************************************************
    *	Date
    **********************************************************/
    class Date
    {
    public:
                Date()										{ }
                Date(u32 year, u32 month, u32 day)			{ setYMD(year, month, day); }

        void	setNow ();
        void	setYMD (u32 year, u32 month, u32 day)		{ setDay(day); setMonth(month); setYear(year); }
        void	setDay(u32 t) 								{ assert(t<32);		ts &= 0xFFFFFF00; ts |= t; }
        void	setMonth(u32 t) 							{ assert(t<13);		ts &= 0xFFFF00FF; ts |= (t<<8); }
        void	setYear(u32 t) 								{ assert(t<u16MAX); ts &= 0x0000FFFF; ts |= (t<<16); }

        u32		getDay() const								{ return ((ts & 0x000000FF)); }
        u32		getMonth() const							{ return ((ts & 0x0000FF00) >> 8); }
        u32		getYear() const								{ return ((ts & 0xFFFF0000) >> 16); }
        u32		getYMD() const								{ return ts; }

        void	getYYYYMMDD(char *out, u32 sizeofout) const;

        u32		getInternalRappresentation() const			{ return ts; }
        void	setFromInternalRappresentation(u32 u)		{ ts = u; }

    private:
        u32		ts;


    };

};
#endif // _rheaDate_h_
