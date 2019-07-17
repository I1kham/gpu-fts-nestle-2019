#ifndef _rheaTime24_h_
#define _rheaTime24_h_
#include "rheaDataTypes.h"

namespace rhea
{
    /**********************************************************
    *	Time24
    *	Gestisce un orario composto da hh:mm::ss::msec
    *	Superate le 23:59:59:999 riparte da 00:00:00:0000
    *
    *	5bit	per hour
    *	6bit	per	min
    *	6bit	per sec
    *	10bit	per msec
    **********************************************************/
    class Time24
    {
    public:
                Time24()									{ }
                Time24(u32 h, u32 m,u32 s, u32 ms=0)		{ setHMS(h,m,s,ms); }

        void	setNow ();
        void	setHMS (u32 h, u32 m,u32 s, u32 ms=0)		{ setHour(h); setMin(m); setSec(s); setMSec(ms); }
        void	setHour(u32 t) 								{ assert(t<24);   ts &= (~0x07C00000); ts |= (t<<22); }
        void	setMin(u32 t) 								{ assert(t<60);   ts &= (~0x003F0000); ts |= (t<<16); }
        void	setSec(u32 t) 								{ assert(t<60);   ts &= (~0x0000FC00); ts |= (t<<10); }
        void	setMSec(u32 t) 								{ assert(t<1000); ts &= (~0x000003FF); ts |= t; }

        u8		getHour() const								{ return (u8)((ts & 0x07C00000) >> 22); }
        u8		getMin() const								{ return (u8)((ts & 0x003F0000) >> 16); }
        u8		getSec() const								{ return (u8)((ts & 0x0000FC00) >> 10); }
        u8		getMSec() const								{ return (u8)((ts & 0x000003FF)); }
        void	getHHMMSS(char *out, u32 sizeofout) const;

        void	add (const Time24 &b)						{ add (b.getHour(), b.getMin(), b.getSec(), b.getMSec()); }
        void	add (u32 h, u32 m, u32 s, u32 ms);
        void	sub (const Time24 &b)						{ sub (b.getHour(), b.getMin(), b.getSec(), b.getMSec()); }
        void	sub (u32 h, u32 m, u32 s, u32 ms);
                    //sub = this - b. Se b > this, allora this = 0

        u64		calcTimeInMSec () const						{ u64 ret = getMSec() + 1000*getSec() + 60000*getMin() +3600000*getHour(); return ret; }
        void	setFromMSec (u64 msec);


        u32		getInternalRappresentation() const			{ return ts; }
        void	setFromInternalRappresentation(u32 u)		{ ts = u; }

    private:
        u32		ts;


    };

} // namespace rhea


#endif // _rheaTime24_h_
