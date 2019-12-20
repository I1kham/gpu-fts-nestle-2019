#ifndef _os_h_
#define _os_h_
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


/***********************************************
 * socket
 */
typedef struct sOSNetAddr
{
	sockaddr_in		addr;

	sOSNetAddr&		operator= (const sOSNetAddr& b)							{ memcpy(&addr, &b.addr, sizeof(addr)); return *this; }
} OSNetAddr;


/********************************************************************
 * OSWaitableGrp
 *
 * E' un oggetto che accetta altri oggetti (di tipo socket e/o event) e poi espone una funzione
 * wait() che è in grado di sospendere l'esecuzione fino a che uno (o più) qualunque degli oggetti che gli
 * sono stati "addati" non genera un evento.
 *
 * Nel caso degli OSEvent, è sufficiente chiamare il relativo metodo fire() per far scattare l'evento.
 * Nel caso di OSSocket, l'evento scatta quando ci sono dei dati pronti per essere read(), o quando la socket viene disconnessa.
 *
 */
#ifdef LINUX
    #include "linux/linuxOSWaitableGrp.h"
#endif
#ifdef WIN32
    #include "win/winOSWaitableGrp.h"
#endif
#endif //_os_h_
