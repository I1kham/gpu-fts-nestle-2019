#include <stdio.h>
#include "../rheaString.h"
#include "../rheaUtils.h"
#include "ProtocolServer.h"
#include "ProtocolConsole.h"
#include "ProtocolWebsocket.h"

using namespace rhea;

//****************************************************
ProtocolServer::ProtocolServer(u8 maxClientAllowed, rhea::Allocator *allocatorIN)
{
    logger = &nullLogger;
    allocator = allocatorIN;
    OSSocket_init (&sok);
    clientList.setup (allocator, maxClientAllowed);
}

//****************************************************
ProtocolServer::~ProtocolServer()
{
    this->close();
    clientList.unsetup();
}

//****************************************************
eSocketError ProtocolServer::start (u16 portNumber)
{
    logger->log ("ProtocolServer::start() on port %d... ", portNumber);
    logger->incIndent();

    eSocketError err = OSSocket_openAsTCPServer(&sok, portNumber);
    if (err != eSocketError_none)
    {
        logger->log ("FAIL, error code=%d\n", err);
        logger->decIndent();
        return err;
    }
    logger->log ("OK\n");

    OSSocket_setReadTimeoutMSec  (sok, 0);
    OSSocket_setWriteTimeoutMSec (sok, 10000);

    logger->log ("listen... ");
    if (!OSSocket_listen(sok))
    {
        logger->log ("FAIL\n", err);
        logger->decIndent();
        OSSocket_close(sok);
        return eSocketError_errorListening;
    }
    logger->log ("OK\n");


    //aggiungo la socket al gruppo di oggetti in osservazione
    waitableGrp.addSocket (sok, u32MAX);

    //setup dell'handle array
    clientList.reset();
    u32 nMaxClient = clientList.getNAllocatedElem();
    handleArray.setup (allocator, nMaxClient);

    logger->log ("Done!\n");
    logger->decIndent();
    return eSocketError_none;
}

//****************************************************
void ProtocolServer::close()
{
    logger->log ("ProtocolServer::close()\n");
    logger->incIndent();

    //free dei client ancora connessi
    logger->log ("free of client list\n");
    while (clientList.getNElem())
    {
        HWebsokClient h = clientList(0);
        client_sendClose(h);
    }
    clientList.reset();

    waitableGrp.removeSocket(sok);

    logger->log ("closing socket\n");
    OSSocket_close(sok);

    logger->log ("handleArray.unsetup");
    handleArray.unsetup();

    logger->log ("Done!\n");
    logger->decIndent();
}

/****************************************************
 * wait
 *
 * resta in attesa per un massimo di timeoutMSec e si sveglia quando
 * la socket riceve qualcosa, oppure quando uno qualunque degli oggetti
 * della waitableGrp si sveglia
 */
u8 ProtocolServer::wait (u32 timeoutMSec)
{
    nEvents = 0;
    u8 n = waitableGrp.wait (timeoutMSec);
    if (n == 0)
        return 0;

    //scanno gli eventi perchè non tutti vanno ritornati, ce ne sono alcuni che devo gesire da me
    for (u8 i=0; i<n; i++)
    {
        if (waitableGrp.getEventOrigin(i) == OSWaitableGrp::evt_origin_osevent)
        {
            //un OSEvent è stato fired(), lo segnalo negli eventi che ritorno
            eventList[nEvents].evtType = ProtocolServer::evt_osevent_fired;
            eventList[nEvents++].data.osEvent = &waitableGrp.getEventSrcAsOSEvent(i);
            continue;
        }

        else if (waitableGrp.getEventOrigin(i) == OSWaitableGrp::evt_origin_socket)
        {
            //l'evento è stato generato da una socket
            if (waitableGrp.getEventUserParamAsU32(i) == u32MAX)
            {
                //se la socket è quella del server, allora gestiamo l'eventuale incoming connection
                HWebsokClient clientHandle;
                if (priv_checkIncomingConnection (&clientHandle))
                {
                    eventList[nEvents].evtType = ProtocolServer::evt_new_client_connected;
                    eventList[nEvents++].data.clientHandleAsU32 = clientHandle.asU32();
                }
            }
            else
            {
                //altimenti la socket che si è svegliata deve essere una dei miei client già connessi, segnalo
                //e ritorno l'evento
                const u32 clientHandleAsU32 = waitableGrp.getEventUserParamAsU32(i);
                eventList[nEvents].evtType = ProtocolServer::evt_client_has_data_avail;
                eventList[nEvents++].data.clientHandleAsU32 = clientHandleAsU32;
            }
            continue;
        }
    }

    return nEvents;
}


//****************************************************
void ProtocolServer::client_sendClose (const HWebsokClient hClient)
{
    sRecord *r = NULL;
    if (!handleArray.fromHandleToPointer(hClient, &r))
    {
        //l'handle è invalido!
        return;
    }

    priv_onClientDeath (r);
}

//****************************************************
void ProtocolServer::priv_onClientDeath (sRecord *r)
{
    HWebsokClient hClient = r->handle;
    logger->log ("ProtocolServer::priv_onClientDeath(), client[0x%02X] death\n", hClient.asU32());

    OSSocket_close (r->clientSok);
    waitableGrp.removeSocket (r->clientSok);

    clientList.findAndRemove(hClient);

    RHEADELETE(allocator, r->client);
    handleArray.dealloc(hClient);
}

//****************************************************
ProtocolServer::eEventType ProtocolServer::getEventType (u8 iEvent) const
{
    assert (iEvent < nEvents);
    return eventList[iEvent].evtType;
}

//****************************************************
OSEvent* ProtocolServer::getEventSrcAsOSEvent(u8 iEvent) const
{
    assert (iEvent < nEvents);
    assert (eventList[iEvent].evtType == ProtocolServer::evt_osevent_fired);
    return eventList[iEvent].data.osEvent;
}

//****************************************************
HWebsokClient ProtocolServer::getEventSrcAsClientHandle(u8 iEvent) const
{
    assert (iEvent < nEvents);
    assert (eventList[iEvent].evtType >= 100);

    HWebsokClient h;
    h.initFromU32(eventList[iEvent].data.clientHandleAsU32);
    return h;
}

//****************************************************
bool ProtocolServer::priv_checkIncomingConnection (HWebsokClient *out_clientHandle)
{
    logger->log ("ProtocolServer::priv_checkIncomingConnection()\n");
    logger->incIndent();
    bool ret = priv_checkIncomingConnection2(out_clientHandle);
    logger->decIndent();
    return ret;
}

//****************************************************
bool ProtocolServer::priv_checkIncomingConnection2 (HWebsokClient *out_clientHandle)
{
    OSSocket clientSok;
	if (!OSSocket_accept(sok, &clientSok))
    {
        logger->log("accept failed\n");
		return false;
    }

    //ok, ho un client che si vuole connettere, dovrei ricevere l'handshake
    const u16 BUFFER_SIZE = 2048;
    char buffer[BUFFER_SIZE];

    //attendo un tot in modo da ricevere una comunicazione dalla socket appena connessa.
    //I dati che leggo devono essere un valido handshake per uno dei protocolli supportati
    i32 nBytesRead = OSSocket_read (clientSok, buffer, BUFFER_SIZE, 5000);

    if (nBytesRead == 0)
    {
        logger->log("timeout waiting for handshake. Closing connection to the client\n");
        OSSocket_close (clientSok);
        return false;
    }

    eClientType clientType = eClientType_unknown;
    while (1)
    {
        //Se è un client websocket, gestiamo il suo handshake
        if (ProtocolWebsocket::server_isValidaHandshake(buffer, nBytesRead, clientSok) > 0)
        {
            logger->log ("it's a websocket\n");
            clientType = eClientType_websocket;
            break;
        }

        //Se è un client console, abbiamo un semplice handshake
        if (ProtocolConsole::server_isValidaHandshake(buffer, nBytesRead, clientSok) > 0)
        {
            logger->log ("it's a console\n");
            clientType = eClientType_console;
            break;
        }

        //errore
        logger->log ("ERR: client handsake is invalid, closing connection\n");
        OSSocket_close (clientSok);
        return false;
    }

    assert (clientType != eClientType_unknown);

    //genero un handle per il client
    sRecord *r = handleArray.allocIfYouCan();
    if (NULL == r)
    {
        logger->log ("connection was accepted but we have no more free handle, closing client socket\n");
        rhea::logger->logWarn ("ProtocolServer::checkIncomingConnection -> connection was accepted but we have no more free handle") << rhea::Logger::EOL;
        OSSocket_close (clientSok);
        return false;
    }

    //alloco il client
    r->clientSok = clientSok;
    switch (clientType)
    {
    case eClientType_console:
        r->client = RHEANEW(allocator, ProtocolConsole) (allocator);
        break;

    case eClientType_websocket:
        r->client = RHEANEW(allocator, ProtocolWebsocket) (allocator);
        break;

    default:
        logger->log ("ERR: client type is invalid [%d]\n", clientType);
        handleArray.dealloc(r->handle);
        return false;
    }

    *out_clientHandle = r->handle;
    waitableGrp.addSocket (r->clientSok, r->handle.asU32());
    clientList.append(r->handle);

    logger->log ("Done!\n");
    return true;
}

//****************************************************
i32 ProtocolServer::client_read (const HWebsokClient hClient, rhea::LinearBuffer *out_buffer)
{
    sRecord *r = NULL;
    if (!handleArray.fromHandleToPointer(hClient, &r))
    {
        //l'handle è invalido!
        return 0;
    }

    u8 bSocketWasClosed = 0;
    u16 ret = r->client->read (r->clientSok, out_buffer, &bSocketWasClosed);

    if (bSocketWasClosed)
    {
        priv_onClientDeath(r);
        return -1;
    }

    return ret;
}


//****************************************************
i16 ProtocolServer::client_writeText (const HWebsokClient hClient, const char *strIN) const
{
    sRecord *r = NULL;
    if (!handleArray.fromHandleToPointer(hClient, &r))
    {
        //l'handle è invalido!
        return 0;
    }

    return r->client->writeText (r->clientSok, strIN);
}

//****************************************************
i16 ProtocolServer::client_writeBuffer (const HWebsokClient hClient, const void *bufferIN, u16 nBytesToWrite) const
{
    sRecord *r = NULL;
    if (!handleArray.fromHandleToPointer(hClient, &r))
    {
        //l'handle è invalido!
        return 0;
    }

    return r->client->writeBuffer (r->clientSok, bufferIN, nBytesToWrite);
}

//****************************************************
void ProtocolServer::client_sendPing (const HWebsokClient hClient) const
{
    sRecord *r = NULL;
    if (!handleArray.fromHandleToPointer(hClient, &r))
    {
        //l'handle è invalido!
        return;
    }

    r->client->sendPing(r->clientSok);
}



