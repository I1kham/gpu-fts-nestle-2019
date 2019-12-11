/******************************************************************************************************************
 * In generale, le funzioni contenute in questo header non sono da chiamare direttamente.
	Esiste sicuramente una funzione nel namespace rhea che a sua volta le utilizza.
	Ad esempio, per sapere quanti msec sono trascorsi dall'inizio dell'applicazione, usa
		rhea::getTimeNowMSec() e non chiamare direttamente OS_getTimeNowMSec().

	Vuoi sapere che giorno è oggi?
	Usa rhea::Date::getDateNow() e non direttamente OS_getDateNow()

	Le funzioni qui definite sono tutte quelle che ogni piattaforma deve supportare per poter mantenere il codice piattaforma indipendente.
	I file in OS/win ad esempio implementato queste fn per windows
	I file in OS/linux lo fanno per linux
 */

#ifndef _os_h_
#define _os_h_
#include "OSInclude.h"
#include "OSEnum.h"

/*********************************************************
 * uso interno (usate da rhea::init())
 */
inline bool         internal_OSInit (void *platformSpecificData, const char *appName)													{ return platform::internal_init(platformSpecificData, appName); }
inline void         internal_OSDeinit ()																								{ platform::internal_deinit(); }


/*********************************************************
 *  varie
 */
inline void*        OS_alignedAlloc (size_t alignment, size_t size)																		{ return platform::alignedAlloc (alignment, size); }
inline void			OS_alignedFree(void *p)																								{ platform::alignedFree(p); }

inline const char*  OS_getAppPathNoSlash ()																								{ return platform::getAppPathNoSlash(); }
inline const char*  OS_getPhysicalPathToWritableFolder()																				{ return platform::getPhysicalPathToWritableFolder(); }

/*********************************************************
 * High definition timer
 */
inline uint64_t     OS_getTimeNowMSec()																									{ return platform::getTimeNowMSec(); }


/***********************************************
 * date and time
 */
inline void         OS_getDateNow (u16 *out_year, u16 *out_month, u16 *out_day)															{ platform::getDateNow(out_year, out_month, out_day); }
inline void         OS_getTimeNow (u8 *out_hour, u8 *out_min, u8 *out_sec)																{ platform::getTimeNow(out_hour, out_min, out_sec); }



/***********************************************
 * shell stuff
 */
inline void         OS_runShellCommandNoWait (const char *cmd)																			{ platform::runShellCommandNoWait(cmd); }

/*********************************************************
 * File system
 */
inline bool			OS_FS_DirectoryCreate(const char *path)																				{ return platform::FS_DirectoryCreate(path); }
inline bool			OS_FS_DirectoryDelete(const char *path)																				{ return platform::FS_DirectoryDelete(path); }
inline bool			OS_FS_DirectoryExists(const char *path)																				{ return platform::FS_DirectoryExists(path); }

inline bool			OS_FS_fileExists(const char *filename)																				{ return platform::FS_fileExists(filename); }
inline bool			OS_FS_fileDelete(const char *filename)																				{ return platform::FS_fileDelete(filename); }
inline bool			OS_FS_fileRename(const char *oldFilename, const char *newFilename)													{ return platform::FS_fileRename(oldFilename, newFilename); }

inline bool			OS_FS_findFirstHardDrive(OSDriveEnumerator *h, rheaFindHardDriveResult *out)										{ return platform::FS_findFirstHardDrive(h, out); }
inline bool			OS_FS_findNextHardDrive(OSDriveEnumerator &h, rheaFindHardDriveResult *out)											{ return platform::FS_findNextHardDrive(h, out); }
inline void			OS_FS_findCloseHardDrive(OSDriveEnumerator &h)																		{ return platform::FS_findCloseHardDrive(h); }



/***********************************************
 * thread stuff
 */
inline eThreadError OS_createThread (OSThread &out_handle, OSThreadFunction threadFunction, size_t stackSizeInKb, void *userParam)		{ return platform::createThread (out_handle, threadFunction, stackSizeInKb, userParam); }
inline void         OS_killThread (OSThread &handle)																					{ platform::killThread(handle); }
inline void         OS_waitThreadEnd (OSThread &handle)																					{ platform::waitThreadEnd(handle); }
inline void			OS_sleepMSec (size_t msec)																							{ platform::sleepMSec(msec); }



/**************************************************************************
 * OSEvent
 *
 *	Fornisce un meccanismo multithread per segnalare eventi.
 *  Un thread può ascolare lo stato dell'evento usando OSEvent_wait().
 *  Quando qualcuno chiama OSEvent_fire(), la OSEvent_wait() esce con true.
 *	Per cominciare ad utilizzare un evento, è necessario chiamare una sola volta OSEvent_open.
 *	Fintanto che l'evento è "open", allora è possibile usare fire/wait n volte.
 *	Quando l'evento non è più necessario, chiamare OSEvent_close() per liberare le risorse.
 *
 */
inline void         OSEvent_setInvalid (OSEvent &ev)																					{ platform::event_setInvalid(ev); }
inline bool			OSEvent_isInvalid(const OSEvent &ev)																				{ return platform::event_isInvalid(ev); }
inline bool			OSEvent_isValid(const OSEvent &ev)																					{ return !platform::event_isInvalid(ev); }

inline bool         OSEvent_open (OSEvent *out_ev)																						{ return platform::event_open(out_ev); }
inline void         OSEvent_close (OSEvent &ev)																							{ platform::event_close(ev); }
inline bool			OSEvent_compare(const OSEvent &a, const OSEvent &b)																	{ return platform::event_compare(a, b);  }
inline void         OSEvent_fire (const OSEvent &ev)																					{ platform::event_fire(ev); }
inline bool         OSEvent_wait (const OSEvent &ev, size_t timeoutMSec)																{ return platform::event_wait(ev, timeoutMSec); }
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
inline void            OSCriticalSection_init (OSCriticalSection *cs)																	{ platform::criticalSection_init(cs); }
inline void            OSCriticalSection_close(OSCriticalSection &cs)																	{ platform::criticalSection_close(cs); }
inline bool            OSCriticalSection_enter (OSCriticalSection &cs)																	{ return platform::criticalSection_enter(cs); }
inline void            OSCriticalSection_leave (OSCriticalSection &cs)																	{ platform::criticalSection_leave(cs); }
inline bool            OSCriticalSection_tryEnter (OSCriticalSection &cs)																{ return platform::criticalSection_tryEnter(cs); }


/***********************************************
 * serial port
 */
#include "OSSerialPort.h"



/***********************************************
 * socket
 */
#include "OSSocket.h"

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
