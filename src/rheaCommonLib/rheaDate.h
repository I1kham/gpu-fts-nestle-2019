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
		static void		getDateNow(u16 *out_year, u16 *out_month, u16 *out_day);

    public:
						Date()																{ }
						Date(u32 year, u32 month, u32 day)									{ setYMD(year, month, day); }

		Date&			operator= (const Date& b)											{ ts = b.ts; return *this; }

        void			setNow ();
        void			setYMD (u32 year, u32 month, u32 day)								{ setDay(day); setMonth(month); setYear(year); }
        void			setDay(u32 t) 														{ assert(t<32);		ts &= 0xFFFFFF00; ts |= t; }
        void			setMonth(u32 t) 													{ assert(t<13);		ts &= 0xFFFF00FF; ts |= (t<<8); }
        void			setYear(u32 t) 														{ assert(t<u16MAX); ts &= 0x0000FFFF; ts |= (t<<16); }

        u32				getDay() const														{ return ((ts & 0x000000FF)); }
        u32				getMonth() const													{ return ((ts & 0x0000FF00) >> 8); }
        u32				getYear() const														{ return ((ts & 0xFFFF0000) >> 16); }
        u32				getYMD() const														{ return ts; }

        void			formatAs_YYYYMMDD(char *out, u32 sizeofout, char date_sep = 0x00) const;
		u32				formatAs_YYYYMMDD() const; 
							//questa ritorna un U32 del tipo 20190412

		void			add(const Date &b)													{ addYMD(b.getYear(), b.getMonth(), b.getDay()); }
		void			addYMD(i16 years, i16 months, i16 days);

        u32				getInternalRappresentation() const									{ return ts; }
        void			setFromInternalRappresentation(u32 u)								{ ts = u; }


#ifdef _DEBUG
		static void		debug_test_me();
#endif


    private:
        u32				ts;


    };

};
#endif // _rheaDate_h_
