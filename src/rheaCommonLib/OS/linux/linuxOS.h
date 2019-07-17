#ifdef LINUX
#ifndef _linuxos_h_
#define _linuxos_h_
#include "linuxOSInclude.h"



namespace platform
{
    bool            internal_init (void *platformSpecificData);
    void            internal_deinit ();

    void*           alignedAlloc (size_t alignment, size_t size);
    void            alignedFree (void *p);

    const char*     getAppPathNoSlash ();

    uint64_t        getTimeNowMSec();
    void            sleepMSec (size_t msec);

    void            getDateNow (u16 *out_year, u16 *out_month, u16 *out_day);
    void            getTimeNow (u8 *out_hour, u8 *out_min, u8 *out_sec);

    void            runShellCommandNoWait(const char *cmdIN);

    eThreadError    createThread (OSThread &out_handle, OSThreadFunction threadFunction, size_t stackSizeInKb, void *userParam);
    void            killThread (OSThread &handle);
    void            waitThreadEnd(OSThread &handle);

    bool            event_init (OSEvent *out_ev);
	inline bool		event_compare (const OSEvent &a, const OSEvent &b)								{ return (a.evfd == b.evfd); }
    void            event_close (OSEvent &ev);
    void            event_fire (const OSEvent &ev);
    bool            event_wait (const OSEvent &ev, size_t timeoutMSec);

    inline void     criticalSection_init (OSCriticalSection *cs)                                    { pthread_mutex_init (&cs->cs, NULL); }
    inline void     criticalSection_close (OSCriticalSection &cs)                                   { pthread_mutex_destroy(&cs.cs); }
    inline bool     criticalSection_enter (OSCriticalSection &cs)                                   { return (pthread_mutex_lock(&cs.cs) == 0); }
    inline void     criticalSection_leave (OSCriticalSection &cs)                                   { pthread_mutex_unlock(&cs.cs); }
    inline bool     criticalSection_tryEnter (OSCriticalSection &cs)                                { return (pthread_mutex_trylock(&cs.cs) == 0); }

}   //namespace platform

#include "linuxOSSocket.h"
#include "linuxOSSerialPort.h"


#endif //_linuxos_h_
#endif
