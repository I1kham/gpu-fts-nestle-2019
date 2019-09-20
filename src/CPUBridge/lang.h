/*
	GIX 2019-09-09
	Questo codice è brutto perchè ereditato pari pari dalla precedente versione.
	Per il momento lo lascio cosi', è da riscrivere in futuro in maniera adeguata
*/

#ifndef _LANG_H_
#define _LANG_H_
#include <stdio.h>
#include "../rheaCommonLib/rheaDataTypes.h"

#define	LANG_CHIOCCIOLA			0x0040
#define	LANG_PARAM_SEPARATOR	0x00A7

struct sLanguage
{
    char	iso[4];
    FILE 	*ff[2];
    unsigned char errorCode;
    unsigned char tableIDD[2];
};


void 			lang_init (sLanguage *lang);

void 			lang_open (sLanguage *lang, const char *langISOCode);
const char*		lang_getCurLanguage (const sLanguage *lang);
unsigned char	lang_getErrorCode (const sLanguage *lang);
void            lang_clearErrorCode (sLanguage *lang);
u16				lang_translate (sLanguage *lang, u16 *msgIN_OUT, int maxNumOfXCharInmsgIN_OUT);

#endif // _LANG_H_
