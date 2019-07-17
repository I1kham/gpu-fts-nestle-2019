#include "GUIBridge.h"
#include "GUIBridgeServer.h"

using namespace guibridge;



i16     serverThreadFn (void *userParam);



/**************************************************************************
 * startServer
 *
 *
 */
bool guibridge::startServer (HThreadMsgR *out_hQMessageFromWebserver, HThreadMsgW *out_hQMessageToWebserver)
{
    bool ret = true;
    HThreadMsgR chServerR;
    HThreadMsgW chServerW;
    HThreadMsgR chFromServerToRestOfTheWorldR;
    HThreadMsgW chFromServerToRestOfTheWorldW;

    //creo una coda FIFO da associare al thread del server in modo
    //che sia possibile comunicare con il thread
    rhea::thread::createMsgQ ( &chServerR, &chServerW);

    //creo una coda FIFO che il server utilizza per comunicare eventi al resto del mondo
    rhea::thread::createMsgQ ( &chFromServerToRestOfTheWorldR, &chFromServerToRestOfTheWorldW);


    //crea il thread del server
    rhea::HThread hServer;
    {
        sServerInitParam    init;
        init.hQMessageIN = chServerR;
        init.hQMessageOUT = chFromServerToRestOfTheWorldW;
        rhea::thread::create (&hServer, serverThreadFn, &init);

        //attendo che il thread del server sia partito
        while (1)
        {
            rhea::thread::sleepMSec(100);
            rhea::thread::sMsg msg;
            if (rhea::thread::popMsg(chFromServerToRestOfTheWorldR, &msg))
            {
                if (msg.what == GUIBRIDGE_SERVER_STARTED)
                {
                    if (msg.paramU32 != u32MAX)
                        ret = false;
                }

                rhea::thread::deleteMsg(msg);
                break;
            }
        }
    }

    *out_hQMessageFromWebserver = chFromServerToRestOfTheWorldR;
    *out_hQMessageToWebserver = chServerW;
    return ret;
 }


//*****************************************************************
void guibridge::sendAjaxAnwer (WebsocketServer *server, HWebsokClient &h, u8 requestID, const char *ajaxData, u16 lenOfAjaxData)
{
    const u16 BUFFER_SIZE = 1024;
    u8 buffer[BUFFER_SIZE];

    u16 n = 0;
    buffer[n++] = '#';
    buffer[n++] = 'A';
    buffer[n++] = 'J';
    buffer[n++] = requestID;
    buffer[n++] = 'j';
    buffer[n++] = 'a';

    if (lenOfAjaxData)
    {
        if (n + lenOfAjaxData > BUFFER_SIZE)
        {
            DBGBREAK;
            return;
        }
        memcpy (&buffer[n], ajaxData, lenOfAjaxData);
        n += lenOfAjaxData;
    }

    server->client_writeBuffer (h, buffer, n);
}



/*****************************************************************
 *  prepareEventBuffer
 *
 *  filla *out_buffer con i bytes necessari per inviare l'evento alla GUI.
 *  [in_out_bufferLength] inizialmente deve contenere la dimensione in byte di [out_buffer].
 *
 *  Ritorna true se tutto ok e, in questo caso, filla [in_out_bufferLength] con il numero di byte inseriti in [out_buffer]
 *  Ritorna false se il buffer è troppo piccolo e, in questo caso, filla [in_out_bufferLength] con il numero minimo di byte necessari per [out_buffer]
 */
u8 eventSeqNumber = 0;
bool guibridge::prepareEventBuffer (eEventType eventType, const void *optionalData, u16 lenOfOptionalData, u8 *out_buffer, u16 *in_out_bufferLength)
{
    //calcola la dimensione minima necessaaria in byte per il buffer
    const u16 minSizeInBytes = 8 + lenOfOptionalData;

    if (*in_out_bufferLength < minSizeInBytes)
    {
        DBGBREAK;
        *in_out_bufferLength = minSizeInBytes;
        return false;
    }

    u16 n = 0;
    out_buffer[n++] = '#';
    out_buffer[n++] = 'e';
    out_buffer[n++] = 'V';
    out_buffer[n++] = 'n';
    out_buffer[n++] = (u8)eventType;
    out_buffer[n++] = eventSeqNumber++;

    out_buffer[n++] = ((lenOfOptionalData & 0xFF00) >> 8);
    out_buffer[n++] = (lenOfOptionalData & 0x00FF);
    if (lenOfOptionalData)
    {
        memcpy (&out_buffer[n], optionalData, lenOfOptionalData);
        n += lenOfOptionalData;
    }

    *in_out_bufferLength = n;
    return true;

}


//*****************************************************************
void guibridge::sendEvent (WebsocketServer *server, HWebsokClient &h, eEventType eventType, const void *optionalData, u16 lenOfOptionalData)
{
    const u16 BUFFER_SIZE = 1024;
    u8 buffer[BUFFER_SIZE];

    u16 bufSize = BUFFER_SIZE;
    if (prepareEventBuffer (eventType, optionalData, lenOfOptionalData, buffer, &bufSize))
        server->client_writeBuffer (h, buffer, bufSize);
}




//*****************************************************************
i16 serverThreadFn (void *userParam)
{
    sServerInitParam *init = (sServerInitParam*)userParam;

    guibridge::Server server;
    if (!server.open(2280, init->hQMessageIN, init->hQMessageOUT))
        return -1;

    server.run();
    server.close();
    return 1;
}
