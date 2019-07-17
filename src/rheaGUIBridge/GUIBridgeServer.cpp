#include "GUIBridgeServer.h"
#include "../rheaCommonLib/rheaString.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaLogTargetConsole.h"
#include "GUIBridge.h"
#include "CmdHandler_ajaxReq.h"
#include "CmdHandler_eventReq.h"

using namespace guibridge;


//***************************************************
Server::Server()
{
    localAllocator = NULL;
    server = NULL;
}

//***************************************************
bool Server::open (u16 SERVER_PORT, const HThreadMsgR _hQMessageIN, const HThreadMsgW _hQMessageOUT)
{
    hQMessageIN = _hQMessageIN;
    hQMessageOUT = _hQMessageOUT;

    localAllocator = RHEANEW(rhea::memory_getDefaultAllocator(), rhea::AllocatorSimpleWithMemTrack) ("guib_server");

    server = RHEANEW(localAllocator, WebsocketServer)(8, localAllocator);
    eSocketError err = server->start (SERVER_PORT);
    if (err != eSocketError_none)
    {
        printf ("server> server.start() error %d\n", (int)err);
        rhea::thread::pushMsg (hQMessageOUT, GUIBRIDGE_SERVER_STARTED, (u32)err);
        RHEADELETE(localAllocator, server);
        return false;
    }

    printf ("server> started on port:%d\n", SERVER_PORT);
    rhea::thread::pushMsg (hQMessageOUT, GUIBRIDGE_SERVER_STARTED, u32MAX);



    //lista degli handler dei messaggi
    nextTimePurgeCmdHandlerList = 0;
    cmdHandlerList.setup (localAllocator);

    //linear buffer per la ricezione di msg dai client
    buffer.setup (localAllocator, 128);


    //aggiungo la msgQ di questo thread all'elenco delle cose che il server monitora durante la wait()
    rhea::thread::getMsgQEvent (hQMessageIN, &evFromThreadMsgQ);
    server->addOSEventToWaitList (evFromThreadMsgQ);

    _nextHandlerID = 0;
    return true;
}

//***************************************************
void Server::close()
{
    if (NULL == localAllocator)
        return;

    if (NULL != server)
    {
        buffer.unsetup();
        cmdHandlerList.unsetup();

        server->removeOSEventFromWaitList (evFromThreadMsgQ);
        server->close();
        RHEADELETE(localAllocator, server);
    }

    RHEADELETE(rhea::memory_getDefaultAllocator(), localAllocator);
    printf ("server> closed.\n");
}

//***************************************************
void Server::run()
{
    assert (server != NULL);

    bool bQuit = false;
    while (bQuit == false)
    {
        //la wait() si svegli se è in arrivo una nuova connessione, se un client già connesso invia dati oppure
        //se arriva un msg sulla msgQ di questo thread
        u8 nEvents = server->wait(5000);

        //elimino eventuali handler rimasti pendenti
        u64 timeNowMSec = OS_getTimeNowMSec();
        if (timeNowMSec > nextTimePurgeCmdHandlerList)
        {
            cmdHandlerList.purge (localAllocator, timeNowMSec);
            nextTimePurgeCmdHandlerList = timeNowMSec + 10000;
        }

        //analizzo gli eventi segnalati al server
        for (u8 i=0; i<nEvents; i++)
        {
            switch (server->getEventType(i))
            {
            default:
                printf ("server> unknown event type: %d\n", server->getEventType(i));
                break;

            case WebsocketServer::evt_new_client_connected:
                {
                    HWebsokClient h = server->getEventSrcAsClientHandle(i);
                    printf ("server> new connection accepted (handle:0x%02X)\n", h.asU32());
                }
                break;

            case WebsocketServer::evt_client_has_data_avail:
                priv_onClientHasDataAvail (i);
                break;

            case WebsocketServer::evt_osevent_fired:
                {
                    //E' arrivato un messaggio sulla msgQ di questo thread
                    rhea::thread::sMsg msg;
                    while (rhea::thread::popMsg(hQMessageIN, &msg))
                    {
                        switch (msg.what)
                        {
                        default:
                            DBGBREAK;
                            break;

                        case GUIBRIDGE_GPU_EVENT:
                            priv_onGPUEvent (msg);
                            break;
                        }
                        rhea::thread::deleteMsg(msg);
                    }
                }
                break;
            }
        }
    }
}

/**************************************************************************
 * priv_getANewHandlerID ()
 *
 * Ritorna un ID univoco da assegnare agli handler dei messaggi
 */
u16 Server::priv_getANewHandlerID ()
{
    _nextHandlerID++;

    //i primi RESERVED_HANDLE_RANGE id sono riservati ad uso interno
    if (_nextHandlerID < RESERVED_HANDLE_RANGE)
        _nextHandlerID = RESERVED_HANDLE_RANGE;
    return _nextHandlerID;
}


/**************************************************************************
 * onClientHasDataAvail
 *
 * Un client collegato al server ha inviato dei dati lungo la webSocket
 */
void Server::priv_onClientHasDataAvail (u8 iEvent)
{
    HWebsokClient h = server->getEventSrcAsClientHandle(iEvent);

    i16 nBytesLetti = server->client_read (h, &buffer);
    if (nBytesLetti == -1)
    {
        printf ("client [0x%02X]> connection closed\n", h.asU32());
        return;
    }

    if (nBytesLetti < 1)
        return;

    //ho ricevuto un messaggio
    printf ("client [0x%02X]> received:", h.asU32());
    u8 *p = buffer._getPointer(0);
    /*
    for (i16 t=0; t<nBytesLetti; t++)
        printf(" 0x%02X", p[t]);
    printf("\n");
    */

    //deve essere lungo almeno 7 byte e iniziare con #
    if (nBytesLetti >= 7 && p[0] == '#')
    {
        const u8  commandChar = p[1];
        const u8  requestID = p[2];
        const u16 payloadLen = ((u16)p[3] * 256) + (u16)p[4];

        if (nBytesLetti >= 7 + payloadLen)
        {
            u8 *payload = &p[5];
            u16 ck = ((u16)p[5+payloadLen] * 256) + p[6+payloadLen];

            //verifica della ck
            u16 ckCheck = 0;
            for (u16 i=0; i<nBytesLetti-2; i++)
                ckCheck += p[i];
            if (ckCheck != ck)
                return;

            switch (commandChar)
            {
            default:
                return;

            case 'A':
                //ho ricevuto una richiesta di tipo "ajax"
                //Istanzio un "handler" che possa gestire la richiesta. Se quest'handler esiste, allora lo aggiungo alla lista degli handler
                //attivi e chiamo il suo metodo handleRequestFromGUI() per fargli gestire la richiesta.
                //Generalmente la cosa si traduce nell'handler che, a sua volta, appende una richiesta alla msgQ di uscita del server in modo che la GPU
                //possa intercettare la richiesta e rispondere sulla stessa msgQ usanto un msg con il parametro what==GUIBRIDGE_GPU_EVENT
                //La risposta della GPU (che prevede tra i vari parametri anche l'handlerID), viene gestita dallo stesso handler che ho istanziato qui e ora.
                {
                    const char *params = NULL;
                    CmdHandler_ajaxReq *handler = CmdHandler_ajaxReqFactory::spawn (localAllocator, h, requestID, payload, payloadLen, priv_getANewHandlerID(), 10000, &params);
                    if (NULL != handler)
                    {
                        cmdHandlerList.add (handler);
                        handler->handleRequestFromGUI (hQMessageOUT, params);
                    }
                }
                break;

            case 'E':
                //ho ricevuto una richiesta di tipo "scatena un evento"
                //Funziona allo stesso modo di cui sopra
                if (payloadLen >= 1)
                {
                    guibridge::eEventType evType = (guibridge::eEventType)payload[0];
                    CmdHandler_eventReq *handler =  CmdHandler_eventReqFactory::spawn (localAllocator, h, evType, priv_getANewHandlerID(), 10000);
                    if (NULL != handler)
                    {
                        cmdHandlerList.add (handler);
                        handler->handleRequestFromGUI (hQMessageOUT, payload, payloadLen);
                    }
                }
                break;
            }
        }
    }
}


/**************************************************************************
 * priv_onGPUEvent
 *
 * E' arrivato un messaggio da parte della GPU sulla msgQ di questo thread.
 * Il msg è di tipo msg.what==GUIBRIDGE_GPU_EVENT
 */
void Server::priv_onGPUEvent (rhea::thread::sMsg &msg)
{
    const u8 *data = (const u8*)msg.buffer;
    u16 handlerID = (u16)data[0];
    handlerID<<=8;
    handlerID |= data[1];

    if (handlerID < RESERVED_HANDLE_RANGE)
    {
        //siamo in un caso speciale. L'handler non esiste perchè questa non è una risposta
        //ad una mia richiesta, piuttosto è la GPU che mi ha chiesto di inviare un evento.
        //L'handler  stesso identifica il tipo di richiesta (eEventType)
        eEventType evType = (eEventType)handlerID;

        for (u32 i=0; i<server->client_getNumConnected(); i++)
        {
            HWebsokClient h = server->client_getByIndex(i);
            CmdHandler_eventReq *handler =  CmdHandler_eventReqFactory::spawn (localAllocator, h, evType, 0, 10000);
            if (NULL != handler)
            {
                handler->handleAnswerToGUI (server, &data[2]);
                RHEADELETE(localAllocator, handler);
            }
        }
    }
    else
    {
        //la GPU ha risposta ad una mia richiesta, per cui io devo avere precedentemente spawnato un handler per gestire questa evenienza.
        //Recupero l'handler appropriato e la gestisco
        CmdHandler *handler = cmdHandlerList.findByID (handlerID);
        if (NULL != handler)
        {
            handler->handleAnswerToGUI (server, &data[2]);
            cmdHandlerList.removeAndDeleteByID (localAllocator, handlerID);
        }
    }
}
