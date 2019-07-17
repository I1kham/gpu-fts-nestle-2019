#ifndef _LANG_H_
#define _LANG_H_
#include <stdio.h>

class QChar;

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
void			lang_translate (sLanguage *lang, QChar *msgIN_OUT, int maxNumOfXCharInmsgIN_OUT);

#endif // _LANG_H_
