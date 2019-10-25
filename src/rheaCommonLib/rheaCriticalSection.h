#ifndef _rheaCriticalSection_h_
#define _rheaCriticalSection_h_
#include "OS/OS.h"


/**************************************************************************
 * CriticalSection
 *
 * Fornisce un meccanismo per l'accesso atomico a sezioni di codice.
 * Simile ad un mutex o all'equivalente CriticalSection di windows
 *
 */
namespace rhea
{
	namespace criticalsection
	{
		inline void init(OSCriticalSection *cs)								{ platform::criticalSection_init(cs); }
		inline void close(OSCriticalSection &cs)							{ platform::criticalSection_close(cs); }
		inline bool enter(OSCriticalSection &cs)							{ return platform::criticalSection_enter(cs); }
		inline void leave(OSCriticalSection &cs)							{ platform::criticalSection_leave(cs); }
		inline bool tryEnter(OSCriticalSection &cs)							{ return platform::criticalSection_tryEnter(cs); }

    } //namespace thread

} // namespace rhea



#endif // _rheaCriticalSection_h_
