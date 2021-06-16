#include <stdio.h>
#include "../rheaString.h"
#include "../rheaUtils.h"
#include "ProtocolSocketServer.h"
#include "ProtocolChSocketTCP.h"
#include "ProtocolConsole.h"
#include "ProtocolWebsocket.h"

using namespace rhea;

//****************************************************
ProtocolSocketServer::ProtocolSocketServer(u8 maxClientAllowed, rhea::Allocator *allocatorIN)
{
    logger = &nullLogger;
    allocator = allocatorIN;
	rhea::socket::init (&sok);
	rhea::socket::init(&sokUDP);
    clientList.setup (allocator, maxClientAllowed);
}

//****************************************************
ProtocolSocketServer::~ProtocolSocketServer()
{
    this->close();
    clientList.unsetup();
}

//****************************************************
eSocketError ProtocolSocketServer::start (u16 portNumber)
{
    logger->log ("ProtocolServer::start() on port %d... ", portNumber);
    logger->incIndent();

    eSocketError err = rhea::socket::openAsTCPServer(&sok, portNumber);
    if (err != eSocketError_none)
    {
        logger->log ("FAIL, error code=%d\n", err);
        logger->decIndent();
        return err;
    }
    logger->log ("OK\n");

    rhea::socket::setReadTimeoutMSec  (sok, 0);
    rhea::socket::setWriteTimeoutMSec (sok, 10000);

    logger->log ("listen... ");
    if (!rhea::socket::listen(sok))
    {
        logger->log ("FAIL\n", err);
        logger->decIndent();
        rhea::socket::close(sok);
        return eSocketError_errorListening;
    }
    logger->log ("OK\n");


    //aggiungo la socket al gruppo di oggetti in osservazione
    waitableGrp.addSocket (sok, u32MAX);

    //setup dell'handle array
    clientList.reset();
    u32 nMaxClient = clientList.getNAllocatedElem();
    handleArray.setup (allocator, nMaxClient);


	//apro la socket UDP per il broadcastr hello
    logger->log ("opening UDP port [%d]...", portNumber+1);
	err = rhea::socket::openAsUDP(&sokUDP);
    if (err != eSocketError_none)
        logger->log ("FAILED (open)[%d]\n", err);
    else
    {
        err = rhea::socket::UDPbind (sokUDP, portNumber + 1);
        if (err != eSocketError_none)
            logger->log ("FAILED (bind)[%d]\n", err);
        else
        {
            logger->log ("OK\n");
            //aggiungo la socket al gruppo di oggetti in osservazione
            waitableGrp.addSocket(sokUDP, u32MAX-1);
        }
    }



    logger->log ("Done!\n");
    logger->decIndent();
    return eSocketError_none;
}

//****************************************************
void ProtocolSocketServer::close()
{
    logger->log ("ProtocolServer::close()\n");
    logger->incIndent();

    //free dei client ancora connessi
    logger->log ("free of client list\n");
    while (clientList.getNElem())
    {
        HSokServerClient h = clientList(0);
        client_sendClose(h);
    }
    clientList.reset();

    waitableGrp.removeSocket(sok);
	waitableGrp.removeSocket(sokUDP);

    logger->log ("closing socket\n");
    rhea::socket::close(sok);
	rhea::socket::close(sokUDP);

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
u8 ProtocolSocketServer::wait (u32 timeoutMSec)
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
            eventList[nEvents].evtType = ProtocolSocketServer::evt_osevent_fired;
			eventList[nEvents].data.if_event.osEvent = &waitableGrp.getEventSrcAsOSEvent(i);
			eventList[nEvents++].data.if_event.userParam = waitableGrp.getEventUserParamAsU32(i);
            continue;
        }

        else if (waitableGrp.getEventOrigin(i) == OSWaitableGrp::evt_origin_socket)
        {
            //l'evento è stato generato da una socket
            if (waitableGrp.getEventUserParamAsU32(i) == u32MAX)
            {
                //se la socket è quella del server, allora gestiamo l'eventuale incoming connection
                HSokServerClient clientHandle;
                if (priv_checkIncomingConnection (&clientHandle))
                {
                    eventList[nEvents].evtType = ProtocolSocketServer::evt_new_client_connected;
                    eventList[nEvents++].data.if_socket.clientHandleAsU32 = clientHandle.asU32();
                }
            }
			else if (waitableGrp.getEventUserParamAsU32(i) == u32MAX - 1)
			{
				//evento generato dalla socket UDP
				const u8 BUFFER_SIZE = 64;
				u8 buffer[BUFFER_SIZE];
				OSNetAddr from;
				u32 nBytesRead = rhea::socket::UDPReceiveFrom(sokUDP, buffer, BUFFER_SIZE, &from);
				if (nBytesRead >= 9)
				{
					if (memcmp(buffer, "RHEAHEllO", 9) == 0)
					{
						//rispondo
						u8 ct = 0;
						buffer[ct++] = 'r';
						buffer[ct++] = 'h';
						buffer[ct++] = 'e';
						buffer[ct++] = 'a';
						buffer[ct++] = 'H';
						buffer[ct++] = 'e';
						buffer[ct++] = 'l';
						buffer[ct++] = 'L';
						buffer[ct++] = 'O';
						rhea::socket::UDPSendTo(sokUDP, buffer, ct, from);
					}
				}
			}
			else
            {
                //altimenti la socket che si è svegliata deve essere una dei miei client già  connessi, segnalo
                //e ritorno l'evento
                const u32 clientHandleAsU32 = waitableGrp.getEventUserParamAsU32(i);
                eventList[nEvents].evtType = ProtocolSocketServer::evt_client_has_data_avail;
                eventList[nEvents++].data.if_socket.clientHandleAsU32 = clientHandleAsU32;
            }
            continue;
        }
    }

    return nEvents;
}


//****************************************************
void ProtocolSocketServer::client_sendClose (const HSokServerClient hClient)
{
    sRecord *r = NULL;
    if (!handleArray.fromHandleToPointer(hClient, &r))
    {
        //l'handle è invalido!
        return;
    }

	r->protocol->close(r->ch);
    priv_onClientDeath (r);
}

//****************************************************
void ProtocolSocketServer::priv_onClientDeath (sRecord *r)
{
    HSokServerClient hClient = r->handle;
	//logger->log("ProtocolServer::priv_onClientDeath()\n");
	//logger->incIndent();
	//logger->log("client [0x%02X]\n", hClient.asU32());

	//marco come "ignora" eventuali eventi in lista che sono relativi a questo client
	for (u8 i = 0; i < nEvents; i++)
	{
		const u16 ev = (u16)eventList[i].evtType;
		if (ev >= evt_new_client_connected && ev <= evt_client_max)
		{
			if (eventList[i].data.if_socket.clientHandleAsU32 == r->handle.asU32())
				eventList[i].evtType = evt_ignore;
		}
	}

	//logger->log("removing socket\n");
	waitableGrp.removeSocket (r->ch->getSocket());

	//logger->log("closing channel\n");
	r->ch->close();

	//logger->log("removing from clientlist\n");
    clientList.findAndRemove(hClient);

	//logger->log("deleting protocol\n");
    RHEADELETE(allocator, r->protocol);

	//logger->log("deleting channel\n");
	RHEADELETE(allocator, r->ch);

	//logger->log("dealloc handle\n");
    handleArray.dealloc(hClient);

	//logger->decIndent();
}

//****************************************************
ProtocolSocketServer::eEventType ProtocolSocketServer::getEventType (u8 iEvent) const
{
    assert (iEvent < nEvents);
    return eventList[iEvent].evtType;
}

//****************************************************
u32 ProtocolSocketServer::getEventSrcUserParam(u8 iEvent) const
{
	assert(iEvent < nEvents);
	return eventList[iEvent].data.if_event.userParam;
}

//****************************************************
OSEvent* ProtocolSocketServer::getEventSrcAsOSEvent(u8 iEvent) const
{
    assert (iEvent < nEvents);
    assert (eventList[iEvent].evtType == ProtocolSocketServer::evt_osevent_fired);
    return eventList[iEvent].data.if_event.osEvent;
}

//****************************************************
HSokServerClient ProtocolSocketServer::getEventSrcAsClientHandle(u8 iEvent) const
{
    assert (iEvent < nEvents);
    assert (eventList[iEvent].evtType >= 100);

    HSokServerClient h;
    h.initFromU32(eventList[iEvent].data.if_socket.clientHandleAsU32);
    return h;
}

//****************************************************
bool ProtocolSocketServer::priv_checkIncomingConnection (HSokServerClient *out_clientHandle)
{
logger->log ("ProtocolServer::priv_checkIncomingConnection()\n");
logger->incIndent();
    bool ret = priv_checkIncomingConnection2(out_clientHandle);
logger->decIndent();
    return ret;
}
bool ProtocolSocketServer::priv_checkIncomingConnection2 (HSokServerClient *out_clientHandle)
{
    OSSocket acceptedSok;
	if (!rhea::socket::accept(sok, &acceptedSok))
    {
        logger->log("ProtocolServer::priv_checkIncomingConnection() => accept failed\n");
		return false;
    }

	ProtocolChSocketTCP	*ch = RHEANEW(allocator, ProtocolChSocketTCP) (allocator, 1024, 8192);
	ch->bindSocket(acceptedSok);



    //ok, ho un client che si vuole connettere, dovrei ricevere l'handshake
    //attendo un tot in modo da ricevere una comunicazione dalla socket appena connessa.
    //I dati che leggo devono essere un valido handshake per uno dei protocolli supportati
	i32 nBytesRead = ch->read(5000);

    if (nBytesRead == 0 || nBytesRead >= protocol::RES_ERROR)
    {
        logger->log("ProtocolServer::priv_checkIncomingConnection() => timeout waiting for handshake. Closing connection to the client\n");
		ch->close();
		RHEADELETE(allocator, ch);
        return false;
    }

	IProtocol *protocol = NULL;
    while (1)
    {
        //Se è un client websocket, gestiamo il suo handshake
        if (ProtocolWebsocket::server_isAValidHandshake(ch->getReadBuffer(), ch->getNumBytesInReadBuffer()))
        {
logger->log ("it's a websocket\n");
			protocol = RHEANEW(allocator, ProtocolWebsocket) (allocator, 1024, 8192);
            break;
        }

        //Se è un client console, abbiamo un semplice handshake
        if (ProtocolConsole::server_isAValidHandshake(ch->getReadBuffer(), ch->getNumBytesInReadBuffer()))
        {
logger->log ("it's a console\n");
			protocol = RHEANEW(allocator, ProtocolConsole) (allocator, 512, 4096);
            break;
        }

        //errore
        logger->log ("ProtocolServer::priv_checkIncomingConnection() => client handsake is invalid, closing connection\n");
		ch->close();
		RHEADELETE(allocator, ch);
        return false;
    }

	assert(protocol != NULL);

	//provo a portare a termine l'handshake
	if (!protocol->handshake_serverAnswer(ch, logger))
	{
		logger->log("ProtocolServer::priv_checkIncomingConnection() =>  client handsake failed, closing connection\n");
		RHEADELETE(allocator, protocol);
		ch->close();
		RHEADELETE(allocator, ch);
		return false;
	}


    //tuttok ok. Genero un handle per il client
    sRecord *r = handleArray.allocIfYouCan();
    if (NULL == r)
    {
        logger->log ("ProtocolServer::priv_checkIncomingConnection() => connection was accepted but we have no more free handle, closing client socket\n");
        rhea::sysLogger->logWarn ("ProtocolServer::checkIncomingConnection -> connection was accepted but we have no more free handle") << rhea::Logger::EOL;
		
		protocol->close(ch);
		RHEADELETE(allocator, protocol);
		
		ch->close();
		RHEADELETE(allocator, ch);
		return false;
	}
	
	r->protocol = protocol;
	r->ch = ch;
	*out_clientHandle = r->handle;

    //waitableGrp.addSocket (r->ch->getSocket(), r->handle.asU32());
	waitableGrp.addSocket(acceptedSok, r->handle.asU32());
    clientList.append(r->handle);

logger->log ("Done!\n");
    return true;
}

//****************************************************
i32 ProtocolSocketServer::client_read (const HSokServerClient hClient, rhea::LinearBuffer &out_buffer)
{
    sRecord *r = NULL;
    if (!handleArray.fromHandleToPointer(hClient, &r))
    {
        //l'handle è invalido!
        return 0;
    }

    u16 ret = r->protocol->read (r->ch, 0, out_buffer);
	if (ret >= protocol::RES_ERROR)
    {
        priv_onClientDeath(r);
        return -1;
    }

	
    return (i32)ret;
}


//****************************************************
i32 ProtocolSocketServer::client_writeBuffer (const HSokServerClient hClient, const u8 *bufferIN, u16 nBytesToWrite)
{
    sRecord *r = NULL;
    if (!handleArray.fromHandleToPointer(hClient, &r))
    {
        //l'handle è invalido!
        return 0;
    }
    
	u16 ret = r->protocol->write (r->ch, bufferIN, nBytesToWrite, 1000);
	if (ret >= protocol::RES_ERROR)
	{
		priv_onClientDeath(r);
		return -1;
	}

	return (i32)ret;
}


