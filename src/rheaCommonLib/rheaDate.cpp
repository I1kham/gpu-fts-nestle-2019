#include "rheaDate.h"
#include "OS/OS.h"

using namespace rhea;


//****************************
void Date::setNow ()
{
    u16	d,m,y;
    OS_getDateNow (&y, &m, &d);
    setYMD (y, m, d);
}

//****************************
void Date::formatAs_YYYYMMDD(char *out, u32 sizeofout, char date_sep) const
{
	if (date_sep == 0x00)
	{
		assert(sizeofout >= 9);
		if (sizeofout >= 9)
			sprintf_s(out, sizeofout, "%04d%02d%02d", getYear(), getMonth(), getDay());
		else
			out[0] = 0;
	}
	else
	{
		assert(sizeofout >= 11);
		if (sizeofout >= 11)
			sprintf_s(out, sizeofout, "%04d%c%02d%c%02d", getYear(), date_sep, getMonth(), date_sep, getDay());
		else
			out[0] = 0;
	}
}
