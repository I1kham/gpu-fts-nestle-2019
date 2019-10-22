#ifdef LINUX
#ifndef _linuxos_h_
#define _linuxos_h_
#include "linuxOSInclude.h"
#include "../../rheaDateTime.h"


namespace platform
{
    bool            internal_init (void *platformSpecificData, const char *appName);
    void            internal_deinit ();

    void*           alignedAlloc (size_t alignment, size_t size);
    void            alignedFree (void *p);

    const char*     getAppPathNoSlash ();
    const char*     getPhysicalPathToWritableFolder();

    uint64_t        getTimeNowMSec();
    void            sleepMSec (size_t msec);

    void            getDateNow (u16 *out_year, u16 *out_month, u16 *out_day);
    void            getTimeNow (u8 *out_hour, u8 *out_min, u8 *out_sec);

    void            runShellCommandNoWait(const char *cmdIN);

    eThreadError    createThread (OSThread &out_handle, OSThreadFunction threadFunction, size_t stackSizeInKb, void *userParam);
    void            killThread (OSThread &handle);
    void            waitThreadEnd(OSThread &handle);

	inline void     event_setInvalid(OSEvent &ev)													{ ev.evfd = -1; }
	inline bool		event_isInvalid(const OSEvent &ev)												{ return (ev.evfd == -1); }
    bool            event_open (OSEvent *out_ev);
	inline bool		event_compare (const OSEvent &a, const OSEvent &b)								{ return (a.evfd == b.evfd); }
    void            event_close (OSEvent &ev);
    void            event_fire (const OSEvent &ev);
    bool            event_wait (const OSEvent &ev, size_t timeoutMSec);

    inline void     criticalSection_init (OSCriticalSection *cs)                                    { pthread_mutex_init (&cs->cs, NULL); }
    inline void     criticalSection_close (OSCriticalSection &cs)                                   { pthread_mutex_destroy(&cs.cs); }
    inline bool     criticalSection_enter (OSCriticalSection &cs)                                   { return (pthread_mutex_lock(&cs.cs) == 0); }
    inline void     criticalSection_leave (OSCriticalSection &cs)                                   { pthread_mutex_unlock(&cs.cs); }
    inline bool     criticalSection_tryEnter (OSCriticalSection &cs)                                { return (pthread_mutex_trylock(&cs.cs) == 0); }

                    //====================================== file system
    bool			FS_DirectoryCreate(const char *path);
    bool			FS_DirectoryDelete(const char *path);
    bool			FS_DirectoryExists(const char *path);

    bool			FS_fileExists(const char *filename);
    bool			FS_fileDelete(const char *filename);
    bool			FS_fileRename(const char *oldFilename, const char *newFilename);

    bool			FS_findFirst (OSFileFind *h, const char *strPathNoSlash, const char *strJolly);
    bool			FS_findNext (OSFileFind &h);
    bool			FS_findIsDirectory (const OSFileFind &h);
    void			FS_findGetFileName (const OSFileFind &h, char *out, u32 sizeofOut);
    const char*		FS_findGetFileName(const OSFileFind &h);
    void			FS_findGetCreationTime (const OSFileFind &h, rhea::DateTime *out);
    void			FS_findGetLastTimeModified (const OSFileFind &h, rhea::DateTime *out);
    void			FS_findClose(OSFileFind &h);

}   //namespace platform

#include "linuxOSSocket.h"
#include "linuxOSSerialPort.h"


#endif //_linuxos_h_
#endif
