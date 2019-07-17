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
void Date::getYYYYMMDD(char *out, u32 sizeofout) const
{
    sprintf_s (out, sizeofout, "%04d%02d%02d", getYear(), getMonth(), getDay());
}
