#include "rheaJSONParser.h"


/* esempi
{"name":"pippo", "number":12}
{"range":"all"}
*/

using namespace rhea;
using namespace rhea::string;

//*****************************************
void handleExtractedJSONValue (const u8 *start, u32 len, bool bNeedToBeEscaped, u8 *out_value, u32 sizeOfOut)
{
    if (len==0)
    {
        out_value[0] = 0x00;
        return;
    }

    if (bNeedToBeEscaped)
    {
        //devo eliminare i \\\" in favore di un semplice \"
        u32 t = 0;
        for (u32 i=0; i<len; i++)
        {
            if (start[i] == '\\')
            {
                if (start[i+1] == '\"')
                    continue;
            }
            out_value[t++] = start[i];
            if (t >= sizeOfOut)
            {
                out_value[sizeOfOut-1] = 0;
                break;
            }
        }
    }
    else
    {
        if (len >= sizeOfOut)
            len = sizeOfOut-1;
        memcpy (out_value, start, len);
        out_value[len] = 0;
    }
}

//*****************************************
bool extractJSONValue (utf8::Iter &srcIN, u8 *out_value, u32 sizeOfOut)
{
    utf8::Iter src = srcIN;
    const u8 *s1 = src.getPointerToCurrentPosition();
    if (NULL == s1)
        return false;

    if (src.getCurChar() == '"')
    {
        bool hasEscapedDoubleQuote = false;

        //il value è racchiuso tra apici. Prendo tutto fino a che non trovo un'altro apice uguale all'apice di apertura
        src.advanceOneChar();
        ++s1;

        UTF8Char cApiceDoppio("\"");
        while (1)
        {
            if (!utf8::advanceUntil (src, &cApiceDoppio, 1))
                //non ho trovato l'opening finale
                return false;

            //ho trovato un apice. Se l'apice è preceduto dal carattere backslash, allora non è l'apice finale!
            const u8 *p = src.getPointerToCurrentPosition();
            p--;

            src.advanceOneChar();
            if (p[0] != '\\')
                break;

            hasEscapedDoubleQuote = true;
        }

        srcIN = src;

        const u8 *s2 = src.getPointerToCurrentPosition();
        assert (s2);
        const u32 numBytes = (u32)(s2-s1) -1;
        handleExtractedJSONValue (s1, numBytes, hasEscapedDoubleQuote, out_value, sizeOfOut);
        return true;
    }
    else
    {
        // l'identificatore non è racchiuso tra apici per cui prendo tutto fino a che non trovo un validClosingChars o fine buffer
        UTF8Char cBNRT[4] = { UTF8Char(" "), UTF8Char("\r"), UTF8Char("\n"), UTF8Char("\t") };

        //il primo char però non deve essere uno spazio
        if (utf8::isOneOfThis (src.getCurChar(), cBNRT, 4))
            return false;

        //avanzo fino al primo spazio, graffa chiusa o virgola
        UTF8Char cTerminators[3] = { UTF8Char(" "), UTF8Char("}"), UTF8Char(",") };;
        if (!utf8::advanceUntil (src, cTerminators, 3))
            return false;

        srcIN = src;

        src.backOneChar();
        const u8 *s2 = src.getPointerToCurrentPosition();

        u32 numBytes = 0;
        if (s2 >= s1)
            numBytes = (u32)(s2-s1) +1;

        handleExtractedJSONValue (s1, numBytes, false, out_value, sizeOfOut);

        if (numBytes==0)
        {
            out_value[0] = 0x00;
        }
        else
        {
            if (numBytes >= sizeOfOut)
                numBytes = sizeOfOut-1;
            memcpy (out_value, s1, numBytes);
            out_value[numBytes] = 0;
        }

        return true;
    }

    return false;
}

/****************************************************
 * parse
 *
 */
bool json::parse (const u8 *s, RheaJSonTrapFunction onValueFound, void *userValue)
{
    const u8 MAX_SIZE_OF_FIELD_NAME = 64;
    u8 fieldName[MAX_SIZE_OF_FIELD_NAME];

    const u16 MAX_SIZE_OF_FIELD_VALUE = 4096;
    u8 fieldValue[MAX_SIZE_OF_FIELD_VALUE];

    utf8::Iter iter1;
    iter1.setup(s);

    //skippa spazi e cerca una graffa aperta
    utf8::toNextValidChar(iter1);
    if (iter1.getCurChar() != '{')
        return false;
    iter1.advanceOneChar();

    while (!iter1.getCurChar().isEOF())
    {
        //skippa spazi e cerca il doppio apice
        utf8::toNextValidChar(iter1);
        if (iter1.getCurChar() != '"')
            return false;

        //estraggo il nome tra apici doppi
        if (!extractJSONValue (iter1, fieldName, MAX_SIZE_OF_FIELD_NAME))
            return false;


        //skippa spazi e cerca un ':'
        utf8::toNextValidChar(iter1);
        if (iter1.getCurChar() != ':')
            return false;
        iter1.advanceOneChar();

        //skippa spazi
        utf8::toNextValidChar(iter1);

        //qui ci deve essere un "value"
        if (!extractJSONValue (iter1, fieldValue, MAX_SIZE_OF_FIELD_VALUE))
            return false;


        if (!onValueFound (fieldName, fieldValue, userValue))
            return true;

        //skippa spazi
        utf8::toNextValidChar(iter1);

        //qui ci deve essere una "," oppure una "}"
        if (iter1.getCurChar() == ',')
        {
            iter1.advanceOneChar();
            continue;
        }

        if (iter1.getCurChar() == '}')
        {
            iter1.advanceOneChar();
            break;
        }

        //errore
        return false;
    }
    return true;
}
