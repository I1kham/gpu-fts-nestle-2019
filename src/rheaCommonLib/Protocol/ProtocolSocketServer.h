#ifndef _rheaProtocolServer_h_
#define _rheaProtocolServer_h_
#include "../rhea.h"
#include "../rheaHandleUID88.h"
#include "../rheaHandleArray.h"
#include "../rheaFastArray.h"
#include "../SimpleLogger/NullLogger.h"
#include "ProtocolChSocketTCP.h"
#include "IProtocol.h"

//handle per la gestione dei client
RHEATYPEDEF_HANDLE88(HSokServerClient);

namespace rhea
{
    /**************************************************************************
     * ProtocolSocketServer
     *
	 *	Apre una socket in listen sulla porta [portNumber] (vedi start) e attende connessioni.
	 *	I client che si connettono possono usare uno qualunque dei protocolli che implementano IProtocol
     */
    class ProtocolSocketServer
    {
    public:
        enum class eEventType: u8
        {
			//eventi da 1 a 99 sono relativi a OSEvent
            osevent_fired = 1,      //un OSEvent è stato fired
			osevent_max   = 99,

			//eventi da 100 a 199 sono relativi a socket di client connessi
            new_client_connected = 100,
            client_has_data_avail = 101,
			client_max = 199,

			ignore  = 0xfe,
            unknown = 0xff
        };

    public:
                            ProtocolSocketServer (u8 maxClientAllowed, rhea::Allocator *allocatorIN);
                            /* AllocatorIN è usato internamente da questa classe per allocare tutto quello che le serve (es: i client, i buffer interni e via dicendo).
                             * Considerando che questa classe non è thread safe, potrebbe valere la pena utilizzare un allocator non thread safe se si vuole guadagnare qualcosina
                             * in termini di performace
                             */

        virtual             ~ProtocolSocketServer();

        void                useLogger (ISimpleLogger *loggerIN)                                                     { if (NULL==loggerIN) logger=&nullLogger; else logger=loggerIN; }
		

        eSocketError        start (u16 portNumber);
        void                close ();

        void                addOSEventToWaitList (const OSEvent evt, u32 userParam=0)                               { waitableGrp.addEvent (evt, userParam); }
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
		u32					getEventSrcUserParam(u8 iEvent) const;
        HSokServerClient    getEventSrcAsClientHandle(u8 iEvent) const;



        i32                 client_read (const HSokServerClient hClient, rhea::LinearBuffer &out_buffer);
                            /* A seguito di un evento "evt_client_has_data_avail" (vedi wait()), chiamare questa fn per flushare i dati.
                             * Nel caso in cui tra i dati letti ci fossero dei messaggi utili, il loro payload viene messi in out_buffer.
                             *
                             * Eventuali messaggi di controllo (come previsti per es dal protocollo websocket), vengono gestiti internamente in totale
                             * autonomia (ping, pong, close..)
                             *
                             * Ritorna il numero di bytes inseriti in out_buffer oppure -1 per indicare che la connessione è stata chiusa
                             */

		i32                 client_writeBuffer(const HSokServerClient hClient, const u8 *bufferIN, u16 nBytesToWrite);
		
		void                client_sendClose (const HSokServerClient hClient);
			   

        u32                 client_getNumConnected() const              { return clientList.getNElem(); }
        HSokServerClient    client_getByIndex (u32 i) const             { return clientList(i); }

    private:
		struct sDataForEvent_event
		{
			OSEvent         *osEvent;
			u32				userParam;
		};

		struct sDataForEvent_socket
		{
			u32             clientHandleAsU32;
		};

        union uEventData
        {
			sDataForEvent_event		if_event;
			sDataForEvent_socket	if_socket;
        };

        struct sEvent
        {
            eEventType  evtType;
            uEventData  data;
        };

        struct sRecord
        {
            IProtocol			*protocol;
			ProtocolChSocketTCP	*ch;
            HSokServerClient	handle;

            void            oneTimeInit()	{}
            void            oneTimeDeinit()	{}
            void            onAlloc()       { protocol = NULL; }
            void            onDealloc()     {}
        };



    private:
        bool                priv_checkIncomingConnection (HSokServerClient *out_clientHandle);
        bool                priv_checkIncomingConnection2 (HSokServerClient *out_clientHandle);
        void                priv_onClientDeath (sRecord *r);

    private:
        Allocator                               *allocator;
        ISimpleLogger                           *logger;
        NullLogger                              nullLogger;
        sEvent                                  eventList[OSWaitableGrp::MAX_EVENTS_HANDLE_PER_CALL];
        HandleArray<sRecord,HSokServerClient>   handleArray;
        FastArray<HSokServerClient>             clientList;
        OSWaitableGrp                           waitableGrp;
        OSSocket                                sok;
		OSSocket								sokUDP;
        u8                                      nEvents;
    };
} //namespace rhea


#endif // _rheaProtocolServer_h_
