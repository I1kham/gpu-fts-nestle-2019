#include "GUIBridgeServer.h"
#include "../rheaCommonLib/rheaString.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaLogTargetConsole.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "GUIBridge.h"
#include "CmdHandler_ajaxReq.h"
#include "CmdHandler_eventReq.h"

using namespace guibridge;


//***************************************************
Server::Server()
{
    localAllocator = NULL;
    server = NULL;
    logger = &nullLogger;
}

//***************************************************
void Server::useLogger (rhea::ISimpleLogger *loggerIN)
{
    if (NULL==loggerIN)
        logger = &nullLogger;
    else
        logger = loggerIN;

    if (NULL != server)
        server->useLogger(loggerIN);
}

//***************************************************
bool Server::open (u16 SERVER_PORT, const HThreadMsgR _hQMessageIN, const HThreadMsgW _hQMessageOUT)
{
    logger->log ("GUIBridgeServer::opening\n");
    logger->incIndent();

    hQMessageIN = _hQMessageIN;
    hQMessageOUT = _hQMessageOUT;

    localAllocator = RHEANEW(rhea::memory_getDefaultAllocator(), rhea::AllocatorSimpleWithMemTrack) ("guib_server");

    server = RHEANEW(localAllocator, rhea::ProtocolServer)(8, localAllocator);
    server->useLogger(logger);

    eSocketError err = server->start (SERVER_PORT);
    if (err != eSocketError_none)
    {
        logger->log("FAIL\n");
        logger->decIndent();
        rhea::thread::pushMsg (hQMessageOUT, GUIBRIDGE_SERVER_STARTED, (u32)err);
        RHEADELETE(localAllocator, server);
        return false;
    }

    logger->log("Started!\n");
    logger->decIndent();
    rhea::thread::pushMsg (hQMessageOUT, GUIBRIDGE_SERVER_STARTED, u32MAX);


    //elenco dei clienti identificati
    identifiedClientList.setup (localAllocator);

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
        //Avviso che sto morendo e poi attendo un po' nella speranza che tutti quelli che sono in ascolto sulla mia coda di uscita becchino
        //il messaggio e capiscano
        rhea::thread::pushMsg (hQMessageOUT, GUIBRIDGE_SERVER_DYING, u32MAX);
        rhea::thread::sleepMSec (1000);

        buffer.unsetup();
        cmdHandlerList.unsetup();
        identifiedClientList.unsetup ();

        server->removeOSEventFromWaitList (evFromThreadMsgQ);
        server->close();
        RHEADELETE(localAllocator, server);
    }

    RHEADELETE(rhea::memory_getDefaultAllocator(), localAllocator);
    logger->log ("GUIBridgeServer::closed\n");
}

//***************************************************
void Server::run()
{
    assert (server != NULL);

    bQuit = false;
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
                logger->log ("GUIBridgeServer::run(), unknown event type: %d\n", server->getEventType(i));
                break;

            case rhea::ProtocolServer::evt_new_client_connected:
                {
                    //un client vuole connettersi. Accetto la socket in ingresso, ma mi riservo di chiudere la
                    //connessione se il prossimo msg che mando è != dal messaggio di identificazione
                    //HWebsokClient h = server->getEventSrcAsClientHandle(i);
                    //printf ("server> new connection accepted (handle:0x%02X)\n", h.asU32());
                }
                break;

            case rhea::ProtocolServer::evt_client_has_data_avail:
                priv_onClientHasDataAvail (i);
                break;

            case rhea::ProtocolServer::evt_osevent_fired:
                {
                    //E' arrivato un messaggio sulla msgQ di questo thread
                    rhea::thread::sMsg msg;
                    while (rhea::thread::popMsg(hQMessageIN, &msg))
                    {
                        if (msg.what == GUIBRIDGE_GPU_EVENT)
                            priv_onGPUEvent (msg);
                        else if ((msg.what & 0xFF00) == 0x0300)
                            priv_onConsoleEvent (msg);

                        rhea::thread::deleteMsg(msg);
                    }
                }
                break;
            }
        }
    }

    cmdHandlerList.purge (localAllocator, u64MAX);
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
        logger->log ("GUIBridgeServer::priv_onClientHasDataAvail(), connection [0x%02X] closed\n", h.asU32());
        return;
    }

    if (nBytesLetti < 1)
        return;

    //ho ricevuto un messaggio
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
            if (ck != rhea::utils::simpleChecksum16_calc(p, nBytesLetti-2))
                return;


            //se il client non si è ancora identificato, lo deve fare adesso, altrimenti lo killo
            const IdentifiedClientList::sInfo *idInfo = identifiedClientList.isKwnownClient(h);
            if (NULL == idInfo)
            {
                if (commandChar != 'W' || payloadLen<8)
                {
                    logger->log ("GUIBridgeServer::priv_onClientHasDataAvail(), killing client [0x%02X] because it's not identified\n", h.asU32());
                    server->client_sendClose(h);
                    return;
                }

                const u8 apiVersion = payload[0];
                const u32 identificationCode = ((u32)payload[1] << 24) | ((u32)payload[2] << 16) | ((u32)payload[3] << 8) | (u32)payload[4];
                bool bIsNewClient;
                if (!identifiedClientList.onClientConnected (h, identificationCode, apiVersion, &bIsNewClient))
                {
                    logger->log ("GUIBridgeServer::priv_onClientHasDataAvail(), killing client [0x%02X] because its identification info are wrong [id:0x%02X][apiv:%d]\n", h.asU32(), identificationCode, apiVersion);
                    server->client_sendClose(h);
                    return;
                }

                if (bIsNewClient)
                    logger->log ("GUIBridgeServer::priv_onClientHasDataAvail(), NEW client [id:0x%02X][apiv:%d] as [0x%02X]\n", identificationCode, apiVersion, h.asU32());
                else
                    logger->log ("GUIBridgeServer::priv_onClientHasDataAvail(), EXISTING client [id:0x%02X][apiv:%d] as [0x%02X]\n",identificationCode, apiVersion, h.asU32());
                return;
            }


            logger->log ("GUIBridgeServer::priv_onClientHasDataAvail(), command [%c] from [id:0x%02X][apiv:%d] as [0x%02X]\n", commandChar, idInfo->identificationCode, idInfo->apiVersion, idInfo->currentWebSocketHandleAsU32);
            switch (commandChar)
            {
            default:
                logger->log ("GUIBridgeServer::priv_onClientHasDataAvail(), unkown command [%c]\n", commandChar);
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

/**************************************************************************
 * priv_onConsoleEvent
 *
 * E' arrivato un messaggio da parte della console sulla msgQ di questo thread.
 * Il msg è di tipo msg.what==GUIBRIDGE_CONSOLE_EVENT_xxx
 */
void Server::priv_onConsoleEvent (rhea::thread::sMsg &msg)
{
    switch (msg.what)
    {
    case GUIBRIDGE_CONSOLE_EVENT_QUIT:
        logger->log ("GUIBridgeServer:: quitting..\n");
        bQuit = true;
        break;

    case GUIBRIDGE_CONSOLE_EVENT_PING:
        {
            HWebsokClient h;
            h.initFromU32 (msg.paramU32);
            logger->log ("GUIBridgeServer:: sending ping to [0x%02X]\n", h.asU32());
            server->client_sendPing(h);
        }
        break;

    case GUIBRIDGE_CONSOLE_EVENT_CLOSE:
        {
            HWebsokClient h;
            h.initFromU32 (msg.paramU32);
            logger->log ("GUIBridgeServer:: sending close to [0x%02X]\n", h.asU32());
            server->client_sendClose(h);
        }
        break;

    case GUIBRIDGE_CONSOLE_EVENT_STRING:
        {
            HWebsokClient h;
            h.initFromU32 (msg.paramU32);
            logger->log ("GUIBridgeServer:: sending sending text msg to [0x%02X]\n", h.asU32());
            server->client_writeBuffer (h, (const void*)msg.buffer, msg.bufferSize);
        }
        break;

    case GUIBRIDGE_CONSOLE_EVENT_CLIENT_LIST:
        {
            logger->log ("GUIBridgeServer::\n");
            logger->incIndent();
                logger->log ("=== IDENTIFIED CLIENT LIST ====\n");
                u32 n = identifiedClientList.getNElem();
                for (u32 i=0; i<n; i++)
                {
                    const IdentifiedClientList::sInfo *info = identifiedClientList.getElemByIndex(i);

                    const float firstTimeRegisteredSec = (float)info->firstTimeRegisteredMSec / 1000.0f;
                    const float lastTimeRegisteredSec = (float)info->lastTimeRegisteredMSec / 1000.0f;
                    logger->log("[id:0x%08X] [apiv:0x%02X] [h:0x%02X] [firstTime:%.1f] [lastTime:%.1f]\n", info->identificationCode, info->apiVersion, info->currentWebSocketHandleAsU32, firstTimeRegisteredSec, lastTimeRegisteredSec);
                }
                logger->log ("\nfound: %d clients\n\n", n);
            logger->decIndent();
        }
        break;

    default:
        logger->log ("GUIBridgeServer:: unknown console message\n");
        break;

    }
}
