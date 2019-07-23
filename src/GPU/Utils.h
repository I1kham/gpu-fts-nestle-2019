#ifndef _utils_h_
#define _utils_h_
#include "../rheaCommonLib/rheaDataTypes.h"


class QChar;
class QFont;


namespace utils
{
    u8                  evalChecksum (unsigned char *buf, unsigned int len);
    char*               QCharToCStr (const QChar *in, char *out);
    void                getRightFontForLanguage (QFont &out, int pointSize, const char *iso2LettersLanguageCode);
}

#endif // _utils_h_
