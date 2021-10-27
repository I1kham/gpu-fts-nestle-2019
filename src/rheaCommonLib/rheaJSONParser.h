#ifndef _rheaJSONParser_h_
#define _rheaJSONParser_h_
#include "rheaString.h"



typedef bool (*RheaJSonTrapFunction) (const u8 *fieldName, const u8 *fieldValue, void *userValue);

namespace rhea
{
    namespace json
    {

        bool        parse (const u8 *s, RheaJSonTrapFunction onValueFound, void *userValue);
                        /*  parsa la stringa s che si suppone essere in formato JSON
                         *  Ogni volta che ha disponbibile una coppia field:value, chiama la [onValueFound]
                         *  Se la [onValueFound] ritorna false, il parsing termina, altrimenti prosegue
                         *
                         * Ritorna false in caso di errore nel parsing
                         */

    } // namespace json
} // namespace rhea

#endif //_rrheaJSONParser_h_
