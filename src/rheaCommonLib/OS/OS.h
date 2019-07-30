#ifndef _os_h_
#define _os_h_
#include "OSInclude.h"
#include "OSEnum.h"

/*********************************************************
 * uso interno (usate da rhea::init())
 */
inline bool         internal_OSInit (void *platformSpecificData)        { return platform::internal_init(platformSpecificData); }
inline void         internal_OSDeinit ()                                { platform::internal_deinit(); }


/*********************************************************
 *  varie
 */
inline void*        OS_alignedAlloc (size_t alignment, size_t size)			{ return platform::alignedAlloc (alignment, size); }
inline void			OS_alignedFree(void *p)									{ platform::alignedFree(p); }

inline const char*  OS_getAppPathNoSlash ()									{ return platform::getAppPathNoSlash(); }


/*********************************************************
 * High definition timer
 */
inline uint64_t     OS_getTimeNowMSec ()                                { return platform::getTimeNowMSec(); }


/***********************************************
 * date and time
 */
inline void         OS_getDateNow (u16 *out_year, u16 *out_month, u16 *out_day)         { platform::getDateNow(out_year, out_month, out_day); }
inline void         OS_getTimeNow (u8 *out_hour, u8 *out_min, u8 *out_sec)              { platform::getTimeNow(out_hour, out_min, out_sec); }



/***********************************************
 * shell stuff
 */
inline void         OS_runShellCommandNoWait (const char *cmd)          { platform::runShellCommandNoWait(cmd); }



/***********************************************
 * thread stuff
 */
inline eThreadError OS_createThread (OSThread &out_handle, OSThreadFunction threadFunction, size_t stackSizeInKb, void *userParam)  { return platform::createThread (out_handle, threadFunction, stackSizeInKb, userParam); }
inline void         OS_killThread (OSThread &handle)                    { platform::killThread(handle); }
inline void         OS_waitThreadEnd (OSThread &handle)                 { platform::waitThreadEnd(handle); }
inline void			OS_sleepMSec (size_t msec)                          { platform::sleepMSec(msec); }



/**************************************************************************
 * OSEvent
 *
 * Fornisce un meccanismo multithread per segnalare eventi.
 * Un thread può ascolare lo stato dell'evento usando wait().
 * Quando qualcuno chiama fire(), la wait() esce con true
 *
 */
inline bool         OSEvent_init (OSEvent *out_ev)                              { return platform::event_init(out_ev); }
inline void         OSEvent_close (OSEvent &ev)                                 { platform::event_close(ev); }
inline bool			OSEvent_compare(const OSEvent &a, const OSEvent &b)			{ return platform::event_compare(a, b);  }
inline void         OSEvent_fire (const OSEvent &ev)                            { platform::event_fire(ev); }
inline bool         OSEvent_wait (const OSEvent &ev, size_t timeoutMSec)        { return platform::event_wait(ev, timeoutMSec); }
                    /* ritorna true se l'evento è stato fired
                     * false se il timeout è scaduto senza che l'evento sia stato fired
                     */


/**************************************************************************
 * OSCriticalSection
 *
 * Fornisce un meccanismo per l'accesso atomico a sezioni di codice.
 * Simile ad un mutex o all'equivalente CriticalSection di windows
 *
 */
inline void            OSCriticalSection_init (OSCriticalSection *cs)           { platform::criticalSection_init(cs); }
inline void            OSCriticalSection_close(OSCriticalSection &cs)           { platform::criticalSection_close(cs); }
inline bool            OSCriticalSection_enter (OSCriticalSection &cs)          { return platform::criticalSection_enter(cs); }
inline void            OSCriticalSection_leave (OSCriticalSection &cs)          { platform::criticalSection_leave(cs); }
inline bool            OSCriticalSection_tryEnter (OSCriticalSection &cs)       { return platform::criticalSection_tryEnter(cs); }


/***********************************************
 * serial port
 */
#include "OSSerialPort.h"



/***********************************************
 * socket
 */
#include "OSSocket.h"


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
