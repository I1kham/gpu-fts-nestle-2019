#include <stdio.h>
#include "WebsocketServer.h"
#include "../rheaString.h"
#include "../rheaUtils.h"


// WebSocket Universally Unique IDentifier
static const char   WS_UUID[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

#define WSSERVER_MAX_CLIENT_ALLOWED 8

//****************************************************
WebsocketServer::WebsocketServer(u8 maxClientAllowed, rhea::Allocator *allocatorIN)
{
    allocator = allocatorIN;
	OSSocket_init(&sok);
    clientList.setup (allocator, maxClientAllowed);
}

//****************************************************
WebsocketServer::~WebsocketServer()
{
    this->close();
    clientList.unsetup();
}

//****************************************************
eSocketError WebsocketServer::start(u16 portNumber)
{
    this->close();

    eSocketError err = OSSocket_openAsTCP(&sok, portNumber);
    if (err != eSocketError_none)
        return err;

    OSSocket_setReadTimeoutMSec  (sok, 0);
    OSSocket_setWriteTimeoutMSec (sok, 10000);

    if (!OSSocket_listen(sok))
    {
        OSSocket_close(sok);
        return eSocketError_errorListening;
    }

    //aggiungo la socket al gruppo di oggetti in osservazione
    waitableGrp.addSocket (sok, u32MAX);

    //setup dell'handle array
    clientList.reset();
    u32 nMaxClient = clientList.getNAllocatedElem();
    handleArray.setup (allocator, nMaxClient);

    return eSocketError_none;
}

//****************************************************
void WebsocketServer::close()
{
    //free dei client ancora connessi
    while (clientList.getNElem())
    {
        HWebsokClient h = clientList(0);
        client_sendClose(h);
    }
    clientList.reset();

    waitableGrp.removeSocket(sok);
    OSSocket_close(sok);

    handleArray.unsetup();
}


//****************************************************
void WebsocketServer::client_sendClose (const HWebsokClient hClient)
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
void WebsocketServer::priv_onClientDeath (sRecord *r)
{
    HWebsokClient hClient = r->handle;
    printf ("server> client[0x%02X] death\n", hClient.asU32());

	OSSocket_close (r->client->sok);
    waitableGrp.removeSocket (r->client->sok);

    clientList.findAndRemove(hClient);

    RHEADELETE(allocator, r->client);
    handleArray.dealloc(hClient);
}

//****************************************************
WebsocketServer::eEventType WebsocketServer::getEventType (u8 iEvent) const
{
    assert (iEvent < nEvents);
    return eventList[iEvent].evtType;
}

//****************************************************
OSEvent* WebsocketServer::getEventSrcAsOSEvent(u8 iEvent) const
{
    assert (iEvent < nEvents);
    assert (eventList[iEvent].evtType == WebsocketServer::evt_osevent_fired);
    return eventList[iEvent].data.osEvent;
}

//****************************************************
HWebsokClient WebsocketServer::getEventSrcAsClientHandle(u8 iEvent) const
{
    assert (iEvent < nEvents);
    assert (eventList[iEvent].evtType >= 100);

    HWebsokClient h;
    h.initFromU32(eventList[iEvent].data.clientHandleAsU32);
    return h;
}

//****************************************************
u8 WebsocketServer::wait (u32 timeoutMSec)
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
            //un OSEvent è stato fired()
            eventList[nEvents].evtType = WebsocketServer::evt_osevent_fired;
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
                    eventList[nEvents].evtType = WebsocketServer::evt_new_client_connected;
                    eventList[nEvents++].data.clientHandleAsU32 = clientHandle.asU32();
                }
            }
            else
            {
                //altimenti la socket che si è svegliata deve essere una dei miei client già connessi
                const u32 clientHandleAsU32 = waitableGrp.getEventUserParamAsU32(i);
                eventList[nEvents].evtType = WebsocketServer::evt_client_has_data_avail;
                eventList[nEvents++].data.clientHandleAsU32 = clientHandleAsU32;
            }
            continue;
        }
    }

    return nEvents;
}


//****************************************************
bool WebsocketServer::priv_checkIncomingConnection (HWebsokClient *out_clientHandle)
{
    OSSocket clientSok;
	if (!OSSocket_accept(sok, &clientSok))
		return false;

    //ok, ho un client che si vuole connettere, dovrei ricevere l'handshake
    const u16 BUFFER_SIZE = 2048;
    char buffer[BUFFER_SIZE];

    //waiting for handshake
    OSSocket_read (clientSok, buffer, BUFFER_SIZE, 5000);

    //ok, vediamo se è valido ed eventualmente filliamo buffer con la risposta da rimandare indietro
    if (!priv_handshake (buffer, buffer))
    {
        //printf ("\tinvalid handshake\n");
        OSSocket_close (clientSok);
        return false;
    }

    //invio indietro il mio handshake
    //printf ("\tsending back handshake\n");
    if (OSSocket_write (clientSok, buffer, strlen((const char*)buffer)) <= 0)
    {
        //printf ("\terr\n");
        OSSocket_close (clientSok);
        return false;
    }

    //printf ("\tconnected!\n");


    //genero un handle per il client
    sRecord *r = handleArray.allocIfYouCan();
    if (NULL == r)
    {
        rhea::logger->logWarn ("WebsocketServer::checkIncomingConnection -> connection was accepted but we have no more free handle") << rhea::Logger::EOL;
        OSSocket_close (clientSok);
        return false;
    }

    //alloco il client
    r->client = RHEANEW(allocator, WebsocketClient) (clientSok, allocator);
    *out_clientHandle = r->handle;

    waitableGrp.addSocket (r->client->sok, r->handle.asU32());
    clientList.append(r->handle);

    return true;
}

//****************************************************
bool WebsocketServer::priv_handshake_check_header(const char *src, const char *header) const
{
    return (strncmp(src, header, strlen(header)) == 0);
}

//****************************************************
void WebsocketServer::priv_handshake_copy_header (char *out, const char *src, u16 maxSizeOfOutInBytes) const
{
    maxSizeOfOutInBytes--;
    u16 i=0;
    while (i<maxSizeOfOutInBytes)
    {
        char c = src[i];
        if (c==0x00 || c=='\r' || c=='\n')
            break;
        out[i++] = c;
    }
    out[i] = 0x00;
}

//****************************************************
bool WebsocketServer::priv_handshake_make_accept(const char *received_key, char *acceptKey, u32 sizeOfAcceptKeyInBytes) const
{
    if (sizeOfAcceptKeyInBytes < 32)
        return false;

    //concat di received_key & WS_UUID
    char concat_key[256+64];
    size_t lenOfReceivedKey = strlen(received_key);
    size_t lenOfWS_UUID = strlen(WS_UUID);
    if (lenOfReceivedKey + lenOfWS_UUID >= sizeof(concat_key)-1)
        return false;

    memset (concat_key, 0, sizeof(concat_key));
    strncpy (concat_key, received_key, sizeof(concat_key));
    strncat (concat_key, WS_UUID, sizeof(WS_UUID));


    //sha-1 della key concatenata
    u8 sha1_key[20];
    rhea::sha1(sha1_key, sizeof(sha1_key), concat_key, strlen(concat_key));

    //converto la sha_key in base 64
    //Mi servono almeno 31 bytes (vedi rhea::base64_howManyBytesNeededForEncoding())
    return rhea::base64_encode ((char*)acceptKey, sizeOfAcceptKeyInBytes, sha1_key, sizeof(sha1_key));
}

//****************************************************
bool WebsocketServer::priv_handshake(char *out, char *in) const
{
    //printf ("incoming handshake:\n\t%s\n\n", in);

    static const char HEADER_UPGRADE[] = "Upgrade: websocket";
    static const char HEADER_CONNECTION[] = "Connection: Upgrade";
    static const char HEADER_CONNECTION2[] = "Connection: keep-alive, Upgrade";
    static const char HEADER_HOST[] = "Host: ";
    static const char HEADER_KEY[] = "Sec-WebSocket-Key: ";
    static const char HEADER_VERSION[] = "Sec-WebSocket-Version: ";
    static const char HEADER_EXTENSION[] = "Sec-WebSocket-Extensions: ";
    static const char HEADER_PROTOCOL[] = "Sec-WebSocket-Protocol: ";


    struct Handshake
    {
        char    resource[32];
        char    host[64];
        char    received_key[256];
        char    extension[256];
        char    protocol[128];
        u8      upgrade;
        u8      connection;
        u32     version;
    } hs;
    memset (&hs, 0, sizeof(Handshake));

    //il buffer di handshake deve iniziare con la classica richiesta HTTP
    if (sscanf((char*)in, "GET %s HTTP/1.1", hs.resource) != 1)
    {
        //printf ("Invalid HTTP GET request.\n");
        return false;
    }


    //parsing dell'handshake ricevuto
    char *token = NULL;
    char *tokState;
    token = strtok_r(in, "\r\n", &tokState);
    while(token)
    {
        if (priv_handshake_check_header(token, HEADER_UPGRADE))
            hs.upgrade = 1;

        else if (priv_handshake_check_header(token, HEADER_CONNECTION))
            hs.connection = 1;

        else if (priv_handshake_check_header(token, HEADER_CONNECTION2))
            hs.connection = 2;

        else if (priv_handshake_check_header(token, HEADER_HOST))
            priv_handshake_copy_header(hs.host, &token[strlen(HEADER_HOST)], sizeof(hs.host));

        else if (priv_handshake_check_header(token, HEADER_KEY))
            priv_handshake_copy_header(hs.received_key, &token[strlen(HEADER_KEY)], sizeof(hs.received_key));

        else if (priv_handshake_check_header(token, HEADER_VERSION))
            hs.version = rhea::string::convert::toU32(&token[strlen(HEADER_VERSION)]);

        else if (priv_handshake_check_header(token, HEADER_EXTENSION))
            priv_handshake_copy_header(hs.extension, &token[strlen(HEADER_EXTENSION)], sizeof(hs.extension));

        else if (priv_handshake_check_header(token, HEADER_PROTOCOL))
            priv_handshake_copy_header(hs.protocol, &token[strlen(HEADER_PROTOCOL)], sizeof(hs.protocol));

        token = strtok_r(NULL, "\r\n", &tokState);
    }

    //se tutto ok, preparo l'handshake di risposta
    if (hs.upgrade && hs.connection && hs.host && hs.received_key && hs.version)
    {
        //calcolo la key di risposta
        char    send_key[32];
        if (priv_handshake_make_accept(hs.received_key, send_key, sizeof(send_key)))
        {
            //printf ("Handshake request from %s:\n\tReceived key: \t%s\n\tSend key: \t%s\n", hs.host, hs.received_key, send_key);

            //fillo il buffer con la risposta da rimandare indietro
            const char *connection = HEADER_CONNECTION;
            if (hs.connection == 2)
                connection = HEADER_CONNECTION2;
            sprintf(out,    "HTTP/1.1 101 Switching Protocols\r\n"\
                            "Upgrade: websocket\r\n"\
                            "%s\r\n"\
                            "Sec-WebSocket-Accept: %s\r\n"\
                            "Sec-WebSocket-Protocol: %s\r\n\r\n",
                            connection, send_key, hs.protocol);
            //printf ("Handshake response: %s\n", out);
            return true;
        }
    }

    return false;
}


//****************************************************
i32 WebsocketServer::client_read (const HWebsokClient hClient, rhea::LinearBuffer *out_buffer)
{
    sRecord *r = NULL;
    if (!handleArray.fromHandleToPointer(hClient, &r))
    {
        //l'handle è invalido!
        return 0;
    }

    u8 bSocketWasClosed = 0;
    u16 ret = r->client->read (out_buffer, &bSocketWasClosed);

    if (bSocketWasClosed)
    {
        priv_onClientDeath(r);
        return -1;
    }

    return ret;
}


//****************************************************
i16 WebsocketServer::client_writeText (const HWebsokClient hClient, const char *strIN) const
{
    sRecord *r = NULL;
    if (!handleArray.fromHandleToPointer(hClient, &r))
    {
        //l'handle è invalido!
        return 0;
    }

    return r->client->writeText (strIN);
}

//****************************************************
i16 WebsocketServer::client_writeBuffer (const HWebsokClient hClient, const void *bufferIN, u16 nBytesToWrite) const
{
    sRecord *r = NULL;
    if (!handleArray.fromHandleToPointer(hClient, &r))
    {
        //l'handle è invalido!
        return 0;
    }

    return r->client->writeBuffer (bufferIN, nBytesToWrite);
}

//****************************************************
void WebsocketServer::client_sendPing (const HWebsokClient hClient) const
{
    sRecord *r = NULL;
    if (!handleArray.fromHandleToPointer(hClient, &r))
    {
        //l'handle è invalido!
        return;
    }

    r->client->sendPing();
}



