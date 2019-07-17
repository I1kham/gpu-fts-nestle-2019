#ifdef WIN32
#ifndef _winos_h_
#define _winos_h_
#include "winOSInclude.h"

namespace platform
{
	bool            internal_init (void *platformSpecificData);
	void            internal_deinit();

	void*           alignedAlloc(size_t alignment, size_t size);
	void			alignedFree(void *p);

	const char*     getAppPathNoSlash();

	uint64_t        getTimeNowMSec();
	void            sleepMSec(size_t msec);

	void            getDateNow(u16 *out_year, u16 *out_month, u16 *out_day);
	void            getTimeNow(u8 *out_hour, u8 *out_min, u8 *out_sec);

	void            runShellCommandNoWait(const char *cmdIN);

	eThreadError    createThread(OSThread &out_handle, OSThreadFunction threadFunction, size_t stackSizeInKb, void *userParam);
	void            killThread (OSThread &handle);
	void            waitThreadEnd(OSThread &handle);

	inline bool     event_init (OSEvent *out_ev)										{ out_ev->h = ::CreateEvent(NULL, false, false, NULL); return true; }
	inline void     event_close (OSEvent &ev)											{ ::CloseHandle(ev.h); }
	inline bool		event_compare(const OSEvent &a, const OSEvent &b)					{ return (a.h == b.h); }
	inline void     event_fire (const OSEvent &ev)										{ ::SetEvent(ev.h); }
	inline bool     event_wait (const OSEvent &ev, size_t timeoutMSec)					{ if (WAIT_OBJECT_0 == ::WaitForSingleObject(ev.h, timeoutMSec)) return true; return false; }

	inline void     criticalSection_init(OSCriticalSection *cs)							{ ::InitializeCriticalSection(&cs->cs); }
	inline void     criticalSection_close(OSCriticalSection &cs)						{ ::DeleteCriticalSection(&cs.cs); }
	inline bool     criticalSection_enter(OSCriticalSection &cs)						{ ::EnterCriticalSection(&cs.cs); return true; }
	inline void     criticalSection_leave(OSCriticalSection &cs)						{ ::LeaveCriticalSection(&cs.cs); }
	inline bool     criticalSection_tryEnter(OSCriticalSection &cs)						{ return (TryEnterCriticalSection(&cs.cs) == TRUE); }

}   //namespace platform

#include "winOSSocket.h"
#include "winOSSerialPort.h"

#endif //_winos_h_
#endif //WIN32
