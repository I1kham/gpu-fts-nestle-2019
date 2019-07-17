#include "Utils.h"
#include <stdio.h>
#include <string.h>
#include <qstring.h>

/*************************************************************************************
 * formatta [price] riempendo [out] con una stringa che rappresenta il numero [price] le cui ultime [numDecimal]
 * cifre sono da intendersi come decimali, seprarati dal resto della crifra dal carattere [decimalPointCharacter]
 */
void utils::formatCurrency (u16 price, u8 numDecimal, char decimalPointCharacter, char *out_s, u16 sizeOfOut)
{
    char s[16];
    if (numDecimal == 0)
        sprintf (s, "%d", price);
    else
    {
        const u16 divisore = numDecimal * 10;

        u16 parteIntera = price / divisore;
        u16 parteDecimale = price - (parteIntera * divisore);

        sprintf (s, "%d%c", parteIntera, decimalPointCharacter);

        char sd[8];
        sprintf (sd, "%d", parteDecimale);
        while (strlen(sd) < numDecimal)
            strcat(sd, "0");

        strcat (s, sd);
    }

    strncpy (out_s, s, sizeOfOut-1);
}

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
