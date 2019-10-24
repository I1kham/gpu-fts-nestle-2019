#include <time.h>
#include "rheaDateTime.h"
#include <string.h>

using namespace rhea;

//*****************************************************
void DateTime::formatAs_YYYYMMDDHHMMSS(char *out, u32 sizeOfOut, char char_between_date_and_time, char date_sep, char time_sep) const
{
	/*
	0123456789 12345678
	YYYYaMMaDDbHHcMMcSS
	*/

	out[0] = 0x00;
	date.formatAs_YYYYMMDD(out, sizeOfOut, date_sep);
	
	u32 n = (u32)strlen(out);
	if (n == 0)
		return;

	u32 nLeft = sizeOfOut - n;
	if (nLeft)
	{
		out[n++] = char_between_date_and_time;
		out[n] = 0;
		nLeft--;
	}

	time.formatAs_HHMMSS(&out[n], nLeft, time_sep);
}

//****************************
void DateTime::addMSec(u64 msec)
{
	u32	h = (u32)(msec / 3600000);
	msec -= h * 3600000;

	u32 m = (u32)(msec / 60000);
	msec -= m * 60000;

	u32 s = (u32)(msec / 1000);
	msec -= s * 1000;

	addYMDHIS(0, 0, 0, (i16)h, (i16)m, (i16)s);
}

//****************************
void DateTime::addYMDHIS(i16 years, i16 months, i16 days, i16 hours, i16 minutes, i16 seconds)
{
	struct tm t;
	t.tm_year = (int)date.getYear() - 1900;
	t.tm_mon = (int)date.getMonth() - 1;
	t.tm_mday = (int)date.getDay();
	t.tm_hour = (int)time.getHour();
	t.tm_min = (int)time.getMin();
	t.tm_sec = (int)time.getSec();
	t.tm_isdst = -1;

	t.tm_mday += days;
	t.tm_mon += months;
	t.tm_year += years;
	t.tm_hour += hours;
	t.tm_min += minutes;
	t.tm_sec += seconds;

	mktime(&t);

	date.setYMD(1900 + (u32)t.tm_year, 1 + (u32)t.tm_mon, (u32)t.tm_mday);
	time.setHMS((u32)t.tm_hour, (u32)t.tm_min, (u32)t.tm_sec);
}