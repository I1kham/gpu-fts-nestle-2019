#include "rheaString.h"
#include "rheaTime24.h"
#include "OS/OS.h"

using namespace rhea;


//******************************
void Time24::setNow ()
{
    u8 h,m,s;
	platform::getTimeNow(&h, &m, &s);
    setHMS (h, m, s, 0);
}

//****************************
void Time24::getTimeNow(u8 *out_hour, u8 *out_min, u8 *out_sec)
{
	platform::getTimeNow(out_hour, out_min, out_sec);
}

//****************************
u32 Time24::formatAs_HHMMSS() const
{
	char s[8];
	formatAs_HHMMSS(s, sizeof(s), 0x00);
	return rhea::string::utf8::toU32((const u8*)s);
}

//****************************
void Time24::formatAs_HHMMSS(char *out, u32 sizeofout, char time_sep) const
{
	if (time_sep == 0x00)
	{
		assert(sizeofout >= 7);
		if (sizeofout >= 7)
			sprintf_s(out, sizeofout, "%02d%02d%02d", getHour(), getMin(), getSec());
		else
			out[0] = 0x00;
	}
	else
	{
		assert(sizeofout >= 9);
		if (sizeofout >= 9)
			sprintf_s(out, sizeofout, "%02d%c%02d%c%02d", getHour(), time_sep, getMin(), time_sep, getSec());
		else
			out[0] = 0x00;
	}
}

//******************************
void Time24::add (u32 h, u32 m, u32 s, u32 ms)
{
    u32	hNow = getHour();
    u32 mNow = getMin();
    u32 sNow = getSec();
    u32 msNow = getMSec();

    msNow +=ms;
    while (msNow >= 1000)
    {
        msNow -= 1000;
        ++sNow;
    }

    sNow+=s;
    while (sNow >= 60)
    {
        sNow-=60;
        ++mNow;
    }

    mNow+=m;
    while (mNow>=60)
    {
        mNow-=60;
        ++hNow;
    }

    hNow += h;
    while (hNow >= 24)
        hNow -= 24;
}

//******************************
void Time24::sub (u32 h, u32 m, u32 s, u32 ms)
{
    u64 m1 = calcTimeInMSec();
    u64 m2 = ms + 1000*s + 60000*m +3600000*h;

    if (m2 < m1)
        m1 -= m2;
    else
        m1 = 0;

    setFromMSec (m1);
}

//******************************
void Time24::setFromMSec (u64 msec)
{
    u32	h = (u32)(msec / 3600000);
    msec -= h *3600000;

    u32 m = (u32)(msec / 60000);
    msec -= m*60000;

    u32 s = (u32)(msec / 1000);
    msec -= s*1000;

    setHMS (h, m, s, (u32)msec);
}
