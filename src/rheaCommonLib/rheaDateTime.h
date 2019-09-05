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
                DateTime()												{ }
                DateTime(u32 year, u32 month, u32 day)					{ date.setYMD(year, month, day); time.setHMS(0,0,0,0);}
                DateTime(u32 year, u32 month, u32 day,
                        u32 h, u32 m, u32 s, u32 ms=0)					{ date.setYMD(year, month, day); time.setHMS(h,m,s,ms); }

        void	setNow()												{ date.setNow(); time.setNow(); }
        u64		getInternalRappresentation() const						{ u64 d = date.getInternalRappresentation(); u64 t=time.getInternalRappresentation(); return ((d<<32) | t); }
        void	setFromInternalRappresentation(u64 u)					{ date.setFromInternalRappresentation ((u32)((u >> 32) & 0x00000000FFFFFFFF)); time.setFromInternalRappresentation ((u32)(u&0x00000000FFFFFFFF)); }

		void	formatAs_YYYYMMDDHHMMSS (char *out, u32 sizeOfOut, char char_between_date_and_time=' ', char date_sep='-', char time_sep=':') const;
				/* ritorna in out una stringa cosi' composta YYYYaMMaDDbHHcMMcSS dove:
					YYYY = anno 4 cifre
					a = carattere di separazione (date_Sep, default ='-')
					MM = mese 2 cifre
					a = carattere di separazione (date_Sep, default ='-')
					DD = giorno 2 cifre
					b = carattere di separazione (char_between_date_and_time, default =' ')
					HH = ora, 2 cifre
					c = carattere di separazione (time_sep, default =':')
					MM = minuti, 2 cifre
					c = carattere di separazione (time_sep, default =':')
					SS = secondi, 2 cifre

					a, b, c possono eventualmente essere 0x00 se non si desidera includerli nella stringa finale
				*/
				
    public:
        Date	date;
        Time24	time;

				//============== static utils
        static	DateTime	Now()										{ DateTime t; t.setNow(); return t; }
    };
} //namespace rhea
#endif // _rheaDateTime_h_
