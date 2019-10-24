#ifndef _rheaEvent_h_
#define _rheaEvent_h_
#include "OS/OS.h"

/**************************************************************************
 * OSEvent
 *
 *	Fornisce un meccanismo multithread per segnalare eventi.
 *  Un thread può ascolare lo stato dell'evento usando rhea::event::wait().
 *  Quando qualcuno chiama rhea::event::fire(), la rhea::event::wait() esce con true.
 *	Per cominciare ad utilizzare un evento, è necessario chiamare una sola volta rhea::event::open() a mo' di inizializzazione.
 *	Fintanto che l'evento è "open", allora è possibile usare fire/wait n volte.
 *	Quando l'evento non è più necessario, chiamare rhea::event::close() per liberare le risorse.
 *
 */
namespace rhea
{
	namespace event
	{
		inline	void        setInvalid (OSEvent &ev)							{ platform::event_setInvalid(ev); }
		inline bool			isInvalid(const OSEvent &ev)						{ return platform::event_isInvalid(ev); }
		inline bool			isValid(const OSEvent &ev)							{ return !platform::event_isInvalid(ev); }

		inline bool         open (OSEvent *out_ev)								{ return platform::event_open(out_ev); }
		inline void         close(OSEvent &ev)									{ platform::event_close(ev); }

		inline bool			compare(const OSEvent &a, const OSEvent &b)			{ return platform::event_compare(a, b); }
			
		inline void         fire(const OSEvent &ev)								{ platform::event_fire(ev); }
			
		inline bool         wait(const OSEvent &ev, size_t timeoutMSec)			{ return platform::event_wait(ev, timeoutMSec); }
							/* ritorna true se l'evento è stato fired
							 * false se il timeout è scaduto senza che l'evento sia stato fired
							 */
    } //namespace event

} // namespace rhea



#endif // #define _rheaEvent_h_

