#ifndef _rheaWaitableFIFO_h_
#define _rheaWaitableFIFO_h_
#include "rheaFIFO.h"
#include "rheaEvent.h"

namespace rhea
{
    /*************************************************************************************
     * WaitableFIFO
     *
     * E' una FIFO thread safe che oltre agli usuali metodi push/pop espone il metodo waitIncomingMessage()
     * che è bloccante per un massimo di [timeoutMSec] msec.
     * waitIncomingMessage ritorna true non appena riceve un messaggio (ovvero non appena qualcuno chiama la push).
     * Ritorna false se, nel tempo [timeoutMSec] msec, non ha ricevuto alcun messaggio
     *
     */
    template<typename T>
    class WaitableFIFO
    {
    public:
                        WaitableFIFO ()																		{ }

		virtual         ~WaitableFIFO()																		{ unsetup();  }

		void			setup(Allocator *allocatorIN)														{ rhea::event::open(&event); fifo.setup(allocatorIN);  }
		void			unsetup()																			{ empty(); fifo.unsetup(); rhea::event::close(event); }

        void            empty()                                                                             { fifo.empty(); rhea::event::fire(event); }

        bool            waitIncomingMessage (size_t timeoutMSec)                                            { return rhea::event::wait (event, timeoutMSec); }

        void            push (const T &data)                                                                { fifo.push(data); rhea::event::fire (event); }

        bool            pop (T *out_data)                                                                   { return fifo.pop(out_data); }



        OSEvent&        getOSEvent()                                                                        { return event; }

    private:
        FIFOts<T>       fifo;
        OSEvent         event;
    };

} // namespace rhea


#endif // _rheaWaitableFIFO_h_

