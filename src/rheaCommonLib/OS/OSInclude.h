#ifndef _osinclude_h_
#define _osinclude_h_
#include "../rheaDataTypes.h"
#include "../rheaEnumAndDefine.h"


//macro per trasformare un puntatore in un intero
#define PTR_TO_INT(thePointer)                      reinterpret_cast<IntPointer>(thePointer)

//macro per ottenere un intero che rappresenta pointerA - pointerB
#define PTR_DIFF_TO_INT(pointerA, pointerB)         (reinterpret_cast<IntPointer>(pointerA) - reinterpret_cast<IntPointer>(pointerB))



#ifdef LINUX
    #include "linux/linuxOS.h"
#endif
#ifdef WIN32
    #include "win/winOS.h"
#endif


#endif //_osinclude_h_
