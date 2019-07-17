#ifndef _utils_h_
#define _utils_h_
#include "../rheaCommonLib/rheaDataTypes.h"


class QChar;


namespace utils
{
    void                formatCurrency (u16 price, u8 numDecimal, char decimalPointCharacter, char *out_s, u16 sizeOfOut);
    u8                  evalChecksum (unsigned char *buf, unsigned int len);
    char*               QCharToCStr (const QChar *in, char *out);
}

#endif // _utils_h_
