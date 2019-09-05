#ifndef _rheaWaitableFIFO_h_
#define _rheaWaitableFIFO_h_
#include "rheaFIFO.h"
#include "OS/OS.h"

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
                        WaitableFIFO (Allocator *allocatorIN) : fifo(allocatorIN)                           { OSEvent_open(&event); }

        virtual         ~WaitableFIFO()                                                                     { OSEvent_close(event); }


        void            empty()                                                                             { fifo.empty(); OSEvent_fire(event); }

        bool            waitIncomingMessage (size_t timeoutMSec)                                            { return OSEvent_wait (event, timeoutMSec); }

        void            push (const T &data)                                                                { fifo.push(data); OSEvent_fire (event); }

        bool            pop (T *out_data)                                                                   { return fifo.pop(out_data); }



        OSEvent&        getOSEvent()                                                                        { return event; }

    private:
        FIFOts<T>       fifo;
        OSEvent         event;
    };

} // namespace rhea


#endif // _rheaWaitableFIFO_h_

