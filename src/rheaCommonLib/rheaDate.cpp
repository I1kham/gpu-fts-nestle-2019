#include <time.h>
#include "rheaDate.h"
#include "rheaString.h"
#include "OS/OS.h"

using namespace rhea;

#ifdef _DEBUG
//****************************
void Date::debug_test_me()
{
	rhea::Date dt;
	dt.setYMD(2019, 9, 17);	dt.addYMD(1, 0, 0); assert(dt.formatAs_YYYYMMDD() == 20200917);
	dt.setYMD(2019, 9, 17);	dt.addYMD(0, 1, 0); assert(dt.formatAs_YYYYMMDD() == 20191017);
	dt.setYMD(2019, 9, 17);	dt.addYMD(0, 0, 1); assert(dt.formatAs_YYYYMMDD() == 20190918);
	dt.setYMD(2019, 9, 17);	dt.addYMD(0, 0, 15); assert(dt.formatAs_YYYYMMDD() == 20191002);
}
#endif

//****************************
void Date::setNow ()
{
    u16	d,m,y;
	platform::getDateNow(&y, &m, &d);
    setYMD (y, m, d);
}

//****************************
void Date::getDateNow(u16 *out_year, u16 *out_month, u16 *out_day)
{
	platform::getDateNow(out_year, out_month, out_day);
}

//****************************
u32 Date::formatAs_YYYYMMDD() const
{
	char s[16];
	formatAs_YYYYMMDD(s, sizeof(s), 0x00);
	return rhea::string::convert::toU32(s);
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

//****************************
void Date::addYMD (i16 years, i16 months, i16 days)
{
	struct tm t;
	t.tm_year = (int)getYear() - 1900;
	t.tm_mon = (int)getMonth() - 1;
	t.tm_mday = (int)getDay();
	t.tm_hour= 1;
	t.tm_min = 1;
	t.tm_sec = 1;
	t.tm_isdst = -1;

	t.tm_mday += days;
	t.tm_mon += months;
	t.tm_year += years;
	
	mktime (&t);

	setYMD(1900 + (u32)t.tm_year, 1 + (u32)t.tm_mon, (u32)t.tm_mday);
}