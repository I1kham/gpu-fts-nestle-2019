/******************************************************************************************************************
    CmdHandler_eventReq

    Classe di base per tutti i comandi di tipo "evento".
    Per una descrizione sommaria di cosa sia un comando "evento", vedi SocketBridge.h

    I comandi evento sono identificati da un codice (1 byte) che distingue un evento da un altro.
    L'elenco dei codici è nella enum eEventType (vedi SocketBridgeEnumAndDefine.h).
    
    L'aggiunta di un nuovo comando evento prevede quindi come primo passo la creazione di una nuova voce nella enum eEventType.

    SocketBridge istanzia le classi CmdHandler_eventReq in 2 scenari differenti:
        1)  quando riceve da un client un particolare messaggio via socket. Nel messaggio è indicato il tipo di evento da istanziare e gli
            eventuali parametri di input.
            In base al "tipo di evento" ricevuto dal messaggio, SocketBridge controlla tutti i CmdHandler_eventReq che conosce fino a che non ne
            trova uno il cui EVENT_TYPE_FROM_SOCKETCLIENT sia == all'eventType inviato via messaggio

        2)  quando riceve una notifica da CPUBridge. In base al tipo di notifica ricevuta, SocketBridge controlla tutti gli eventi che conosce fino
            a che non ne trova uno il cui EVENT_ID_FROM_CPUBRIDGE sia == alla notifica ricevuta


    Ogni nuova classe di comando evento, deve essere aggiunta al file CmdHandler_eventReq.cpp nei metodi spawnFromSocketClientEventType()
    e spawnFromCPUBridgeEventID() usando la macro CHECK.
    Questo serve per fare in modo che, alla ricezione dell'eEventType, SocketBrdige sia automaticamente in grado di creare al volo una classe
    del tipo giusto per l'evento ricevuto senza passare per un mega switch() che include tutti i codici evento.
    Per avere un'idea della cosa, vedi il metodo Server::priv_onClientHasDataAvail2() in SocketBridgeServer.cpp:
        
        CmdHandler_eventReq *handler = CmdHandler_eventReqFactory::spawnFromSocketClientEventType(localAllocator, identifiedClient->handle, evType, priv_getANewHandlerID(), 10000);
        [handler] viene creato al volo in base a [evType] ricevuto
        [handler] è la specifica classe che gestisce gli eventi il cui eEventType è == [evType]

    Alla ricezione di un (valido) comando evento, SocketBridge ha due opzioni, a seconda di come è stato pensato il comando:
        1)  passare l'evento direttamente a CPUBridge
        2)  gestire l'evento internamente

    Nel caso 1), l'evento (ad es: richiesta stato CPU) viene passato direttamente a CPUBridge. E' compito di CPUBridge gestire la richiesta
    ed eventualmente produrre una risposta. La risposta di CPUBridge viene passata indietro a SocketBridge tramite una "notify".
    Alla ricezione di una "notify", SocketBridge cerca il comando evento in grado di gestire tale notifica, lo istanzia (se necessario) e ne chiama
    il metodo onCPUBridgeNotification() per inviare la risposta al client che aveva fatto richiesta, oppure a tutti i client se l'evento è stato
    generato spontaneamente da CPUBridge
    
    Nel caso 2), la gestione dell'evento è tutta interna a SocketBridge, non è richiesta la collaborazione della CPU.
    Un esempio è il msg CmdHandler_eventReqClientList.
    Il client vuole conosce l'elenco dei client connessi a SocketBridge.
    SocketBridge risponde senza interpellare la CPU dato che queste sono informazioni che conosce e gestisce in autonomia.


    Esempio di comando evento di tipo 1:
        CmdHandler_eventReqCPUMessage.h/cpp
        Il client vuole conoscere l'attuale messaggio di CPU.
        Questo comando necessita di essere passato alla CPU dato che SocketBrdige non conosce l'informazione richiesta.
        Nella classe CmdHandler_eventReqCPUMessage quindi:
            1)  static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_cpuMessage;
                Per prima cosa la classe indicare l'eEventType per il quale è stata creata. Tutti i messaggi evento di tipo eEventType_cpuMessage verrano
                quindi gestiti da una istanza di questa classe

            2)	static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE;
                Come seconda cosa, la classe dichiara quale tipo di risposta (notifica) si aspetta di ricevere da CPUBridge in risposta alla richiesta

            3) il metodo needToPassDownToCPUBridge() ritorna true, ad indicare che il comando in questione va passato alla CPU

            4) il metodo passDownRequestToCPUBridge() è implementato e, come nella maggioranza di questi casi, non fa altro che inviare un msg al thread di CPUBridge con la richiesta

            4) il metodo onCPUBridgeNotification() è implementato ed è precisamente il metodo che verrà chiamatto da SocketBrdige quando/se questa riceverà 
                una risposta da CPUBridge.
                La risposta di CPUBridge avviene sotto forma di un messaggio (o notifica) sulla messageQ di socketBridge.
                Tale rispsota sarà quindi una notifica di tipo CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE come indicato al punto 2)
                L'elenco delle define delle notiche inviate da CPUBridge è in SocketBridgeEnumAndDefine.h


    Esempio di comando evento di tipo 2:
        CmdHandler_eventReqClientList.h/cpp
        Il client vuole conoscere l'elenco dei client connessi.
        Questo comando non necessita dell'intervento di CPU quindi:
            1)  static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_reqClientList;
                Per prima cosa la classe indicare l'eEventType per il quale è stata creata. Tutti i messaggi evento di tipo eEventType_reqClientList verrano
                quindi gestiti da una istanza di questa classe

            2)	static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_MAX_ALLOWED;
                Come seconda cosa, la classe dichiara che non si aspetta alcuna risposta da CPU (ovviamente, dato che non invierà alcuna richiesta)

            3) il metodo needToPassDownToCPUBridge() ritorna false in quanto, appunto, il comando viene gestito internamente da SocketBrdige senza l'intervento di CPU
            
            4) il metotdo passDownRequestToCPUBridge() è vuoto dato che non verrà mai chiamato

            5) il metodo onCPUBridgeNotification() è vuoto dato che non verrà mai chiamato
            
            6) il meotodo handleRequestFromSocketBridge() è implementato ed è precisamente quello che viene invocato da SocketBrdige alla ricezione 
                del comando. In questo metodo si fabbrica e si invia la risposta al client.

*/
#ifndef _CmdHandler_eventReq_h_
#define _CmdHandler_eventReq_h_
#include "CmdHandler.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReq
     *
     * Classe di base per la gestione delle richieste "event"
     */
    class CmdHandler_eventReq : public CmdHandler
    {
    public:
                                CmdHandler_eventReq (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                                    CmdHandler(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                                {
                                }

		virtual void            passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen) = 0;
		//virtual void			onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge) = 0;
		//virtual bool			needToPassDownToCPUBridge() const = 0;

        virtual void			handleRequestFromSocketBridge(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM)
								{ 
									//le classi derivate devono implementare questa fn SOLO se il messaggio in questione è di quelli che
									//viene gestito direttamente da socketBridge.
									//Si invece si tratta di un msg che deve essere passato al CPUBrige, i metodi da implementare sono passDownRequestToCPUBridge() e
									//onCPUBridgeNotification()
									DBGBREAK;  
								}

    };

    /*********************************************************
     * CmdHandler_eventReqFactory
     *
     *
     */
    class CmdHandler_eventReqFactory
    {
    public:
        static CmdHandler_eventReq* spawnFromSocketClientEventType (rhea::Allocator *allocator, const HSokBridgeClient &identifiedClientHandle, eEventType eventType, u16 handlerID, u64 dieAfterHowManyMSec);
		static CmdHandler_eventReq* spawnFromCPUBridgeEventID (rhea::Allocator *allocator, const HSokBridgeClient &identifiedClientHandle, u16 eventID, u16 handlerID, u64 dieAfterHowManyMSec);
    };
}//namespace socketbridge

#endif // _CmdHandler_eventReq_h_
