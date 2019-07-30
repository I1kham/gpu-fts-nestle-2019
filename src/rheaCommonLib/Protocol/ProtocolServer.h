#ifndef _ProtocolServer_h_
#define _ProtocolServer_h_
#include "../OS/OSWaitableGrp.h"
#include "../rhea.h"
#include "../rheaHandleUID88.h"
#include "../rheaHandleArray.h"
#include "../rheaFastArray.h"
#include "../SimpleLogger/NullLogger.h"
#include "IProtocol.h"


//handle per la gestione dei client
RHEATYPEDEF_HANDLE88(HWebsokClient)

namespace rhea
{
    /**************************************************************************
     * ProtocolServer
     *
     */
    class ProtocolServer
    {
    public:
        enum eEventType
        {
            evt_osevent_fired = 1,      //un OSEvent è stato fired

            //eventi maggiori o uguali a 100 sono relativi ai client connessi
            evt_new_client_connected = 100,
            evt_client_has_data_avail = 101,

            evt_unknown = 0xff
        };

    public:
                            ProtocolServer (u8 maxClientAllowed, rhea::Allocator *allocatorIN);
                            /* AllocatorIN è usato internamente da questa classe per allocare tutto quello che le serve (es: i client, i buffer interni e via dicendo).
                             * Considerando che questa classe non è thread safe, vale la pena utilizzare un allocator non thread safe se si vuole guadagnare qualcosina
                             * in termini di performace
                             */

        virtual             ~ProtocolServer();

        void                useLogger (ISimpleLogger *loggerIN)                                                     { if (NULL==loggerIN) logger=&nullLogger; else logger=loggerIN; }


        eSocketError        start (u16 portNumber);
        void                close ();

        void                addOSEventToWaitList (const OSEvent evt)                                                { waitableGrp.addEvent (evt); }
        void                removeOSEventFromWaitList (const OSEvent evt)                                           { waitableGrp.removeEvent (evt); }
                            /* aggiunge/rimuove un OSEvent all'elenco degli oggetti osservati dalla wait()
                            */


        u8                  wait (u32 timeoutMSec);
                            /* chiamata bloccante per un max di timeoutMSec:
                             *      per specificare un tempo di wait "infinito" (ie: socket sempre bloccante), usare timeoutMSec=u32MAX
                             *      per indicare il tempo di wait minimo possibile, usare timeoutMSec=0
                             *      tutti gli altri valori sono comunque validi ma non assumono significati particolari.
                             *
                             * La funzione termina se il timeout scade oppure se uno o più eventi sono occorsi.
                             * Ritorna il numero di eventi disponibili.
                             * Per conoscere il tipo di evento e recuperare il "chi" ha generato l'evento, vedi le fn getEvent...()
                             */
        eEventType          getEventType (u8 iEvent) const;
        OSEvent*            getEventSrcAsOSEvent(u8 iEvent) const;
        HWebsokClient       getEventSrcAsClientHandle(u8 iEvent) const;



        i32                 client_read (const HWebsokClient hClient, rhea::LinearBuffer *out_buffer);
                            /* A seguito di un evento "evt_client_has_data_avail" (vedi wait()), chiamare questa fn per flushare i dati.
                             * Nel caso in cui tra i dati letti ci fossero dei messaggi utili, il loro payload viene messi in out_buffer.
                             *
                             * Eventuali messaggi di controllo (come previsti per es dal protocollo websocket), vengono gestiti internamente in totale
                             * autonomia (ping, pong, close..)
                             *
                             * Ritorna il numero di bytes inseriti in out_buffer oppure -1 per indicare che la connessione è stata chiusa
                             */

        void                client_sendClose (const HWebsokClient hClient);


        i16                 client_writeText (const HWebsokClient hClient, const char *strIN) const;
        i16                 client_writeBuffer (const HWebsokClient hClient, const void *bufferIN, u16 nBytesToWrite) const;
        void                client_sendPing (const HWebsokClient hClient) const;


        u32                 client_getNumConnected() const              { return clientList.getNElem(); }
        HWebsokClient       client_getByIndex (u32 i) const             { return clientList(i); }

    private:
        enum eClientType
        {
            eClientType_unknown = 0,
            eClientType_websocket = 1,
            eClientType_console = 2
        };



    private:
        union uEventData
        {
            OSEvent         *osEvent;
            u32             clientHandleAsU32;
        };

        struct sEvent
        {
            eEventType  evtType;
            uEventData  data;
        };

        struct sRecord
        {
            IProtocol       *client;
            HWebsokClient	handle;
            OSSocket        clientSok;

            void            oneTimeInit()	{}
            void            oneTimeDeinit()	{}
            void            onAlloc()       { client = NULL; }
            void            onDealloc()     {}
        };



    private:
        bool                priv_checkIncomingConnection (HWebsokClient *out_clientHandle);
        bool                priv_checkIncomingConnection2 (HWebsokClient *out_clientHandle);
        void                priv_onClientDeath (sRecord *r);

    private:
        Allocator                               *allocator;
        ISimpleLogger                           *logger;
        NullLogger                              nullLogger;
        sEvent                                  eventList[OSWaitableGrp::MAX_EVENTS_PER_CALL];
        HandleArray<sRecord,HWebsokClient>      handleArray;
        FastArray<HWebsokClient>                clientList;
        OSWaitableGrp                           waitableGrp;
        OSSocket                                sok;
        u8                                      nEvents;
    };
} //namespace rhea


#endif // _ProtocolServer_h_
