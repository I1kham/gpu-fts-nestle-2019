#include "rheaTime24.h"
#include "OS/OS.h"

using namespace rhea;


//******************************
void Time24::setNow ()
{
    u8 h,m,s;
    OS_getTimeNow (&h, &m, &s);
    setHMS (h, m, s, 0);
}

//****************************
void Time24::getHHMMSS(char *out, u32 sizeofout) const
{
    sprintf_s (out, sizeofout, "%02d%02d%02d", getHour(), getMin(), getSec());
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
