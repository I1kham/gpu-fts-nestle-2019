#include "Utils.h"
#include <stdio.h>
#include <string.h>
#include <qstring.h>
#include <qfont.h>
#include "header.h"


//*************************************************************************************
u8 utils::evalChecksum (unsigned char *buf, unsigned int len)
{
    u8 res = 0;
    for (u8 i=0; i<len; i++)
        res += buf[i];
    return res;
}

//****************************************************
char* utils::QCharToCStr (const QChar *in, char *out)
{
    int i=0;
    while (in[i] != 0x00)
    {
        out[i] = in[i].toLatin1();
        i++;
    }
    out[i] =0;
    return out;
}

/*****************************************************
 * Font
 * Il font col charset Latin, JP, chinese è "Noto Sans CJK SC", installato di default nella immagine dell'OS 5
 * Il font col charset Hebrew, è il Roboto, installato di default nella immagine dell'OS 5
 */
void utils::getRightFontForLanguage (QFont &out, int pointSize, const char *iso2LettersLanguageCode)
{
    if (strcasecmp(iso2LettersLanguageCode, "HE") == 0)
    {
        DEBUG_MSG   ("FONT: using DejaVu Sans");
        out.setFamily("DejaVu Sans");
        out.setPointSize(pointSize);
    }
    else
    {
        DEBUG_MSG   ("FONT: Noto Sans CJK SC");
        out.setFamily("Noto Sans CJK SC");
        out.setPointSize(pointSize);
    }
}
