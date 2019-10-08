#include "rheaJSONParser.h"


/* esempi
{"name":"pippo", "number":12}
{"range":"all"}
*/

using namespace rhea;
using namespace rhea::string;


//*****************************************
void handleExtractedJSONValue (const char *start, u32 len, bool bNeedToBeEscaped, char *out_value, u32 sizeOfOut)
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
bool extractJSONValue (parser::Iter &srcIN, char *out_value, u32 sizeOfOut)
{
    parser::Iter src = srcIN;
    const char *s1 = src.getCurStrPointer();
    if (NULL == s1)
        return false;

    if (src.getCurChar() == '"')
    {
        bool hasEscapedDoubleQuote = false;

        //il value è racchiuso tra apici. Prendo tutto fino a che non trovo un'altro apice uguale all'apice di apertura
        src.next();
        ++s1;

        while (1)
        {
            if (!advanceUntil (src, "\"", 1))
                //non ho trovato l'opening finale
                return false;

            //ho trovato un apice. Se l'apice è preceduto dal carattere backslash, allora non è l'apice finale!
            const char *p = src.getCurStrPointer();
            p--;

            src.next();
            if (p[0] != '\\')
                break;

            hasEscapedDoubleQuote = true;
        }

        srcIN = src;

        const char *s2 = src.getCurStrPointer();
        assert (s2);
        u32 numChar = (u32)(s2-s1) -1;
        handleExtractedJSONValue (s1, numChar, hasEscapedDoubleQuote, out_value, sizeOfOut);
        return true;
    }
    else
    {
        // l'identificatore non è racchiuso tra apici per cui prendo tutto fino a che non trovo un validClosingChars o fine buffer

        //il primo char però non deve essere uno spazio
        if (parser::IsOneOfThis (src.getCurChar(), " \r\n\t", 4))
            return false;

        if (!advanceUntil (src, " },", 3))
            return false;

        srcIN = src;

        src.prev();
        const char *s2 = src.getCurStrPointer();

        u32 numChar = 0;
        if (s2 >= s1)
            numChar = (u32)(s2-s1) +1;

        handleExtractedJSONValue (s1, numChar, false, out_value, sizeOfOut);

        if (numChar==0)
        {
            out_value[0] = 0x00;
        }
        else
        {
            if (numChar >= sizeOfOut)
                numChar = sizeOfOut-1;
            memcpy (out_value, s1, numChar);
            out_value[numChar] = 0;
        }

        return true;
    }

    return false;
}

/****************************************************
 * parse
 *
 */
bool json::parse (const char *s, RheaJSonTrapFunction onValueFound, void *userValue)
{
    const u8 MAX_SIZE_OF_FIELD_NAME = 64;
    char fieldName[MAX_SIZE_OF_FIELD_NAME];

    const u16 MAX_SIZE_OF_FIELD_VALUE = 4096;
    char fieldValue[MAX_SIZE_OF_FIELD_VALUE];

    parser::Iter iter1;
    iter1.setup(s);

    //skippa spazi e cerca una graffa aperta
    parser::toNextValidChar (iter1);
    if (iter1.getCurChar() != '{')
        return false;
    iter1.next();

    while (!iter1.isEOL())
    {
        //skippa spazi e cerca il doppio apice
        parser::toNextValidChar (iter1);
        if (iter1.getCurChar() != '"')
            return false;

        //estraggo il nome tra apici doppi
        if (!extractJSONValue (iter1, fieldName, MAX_SIZE_OF_FIELD_NAME))
            return false;


        //skippa spazi e cerca un ':'
        parser::toNextValidChar (iter1);
        if (iter1.getCurChar() != ':')
            return false;
        iter1.next();

        //skippa spazi
        string::parser::skip (iter1, " ", 1);

        //qui ci deve essere un "value"
        if (!extractJSONValue (iter1,fieldValue, MAX_SIZE_OF_FIELD_VALUE))
            return false;


        if (!onValueFound (fieldName, fieldValue, userValue))
            return true;

        //skippa spazi
        string::parser::skip (iter1, " ", 1);

        //qui ci deve essere una "," oppure una "}"
        if (iter1.getCurChar() == ',')
        {
            iter1.next();
            continue;
        }

        if (iter1.getCurChar() == '}')
        {
            iter1.next();
            break;
        }

        //errore
        return false;
    }
    return true;
}
