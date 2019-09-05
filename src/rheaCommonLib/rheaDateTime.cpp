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