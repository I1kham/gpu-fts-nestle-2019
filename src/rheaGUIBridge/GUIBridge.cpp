#include "GUIBridge.h"
#include "GUIBridgeServer.h"

using namespace guibridge;

struct sServerInitParam
{
    rhea::ISimpleLogger *logger;
    HThreadMsgR chServerR;
    HThreadMsgW chServerW;
    HThreadMsgR chFromServerToRestOfTheWorldR;
    HThreadMsgW chFromServerToRestOfTheWorldW;
};

i16     serverThreadFn (void *userParam);



/**************************************************************************
 * startServer
 *
 *
 */
bool guibridge::startServer (rhea::HThread *out_hServer, HThreadMsgR *out_hQMessageFromWebserver, HThreadMsgW *out_hQMessageToWebserver, rhea::ISimpleLogger *logger)
{
    bool ret = true;
    sServerInitParam    init;

    //creo una coda FIFO da associare al thread del server in modo
    //che sia possibile comunicare con il thread
    rhea::thread::createMsgQ ( &init.chServerR, &init.chServerW);

    //creo una coda FIFO che il server utilizza per comunicare eventi al resto del mondo
    rhea::thread::createMsgQ ( &init.chFromServerToRestOfTheWorldR, &init.chFromServerToRestOfTheWorldW);


    //crea il thread del server
    init.logger = logger;
    rhea::thread::create (out_hServer, serverThreadFn, &init);

    //attendo che il thread del server sia partito
    while (1)
    {
        rhea::thread::sleepMSec(100);
        rhea::thread::sMsg msg;
        if (rhea::thread::popMsg(init.chFromServerToRestOfTheWorldR, &msg))
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

    *out_hQMessageFromWebserver = init.chFromServerToRestOfTheWorldR;
    *out_hQMessageToWebserver = init.chServerW;
    return ret;
 }


//*****************************************************************
void guibridge::sendAjaxAnwer (rhea::ProtocolServer *server, HWebsokClient &h, u8 requestID, const char *ajaxData, u16 lenOfAjaxData)
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
void guibridge::sendEvent (rhea::ProtocolServer *server, HWebsokClient &h, eEventType eventType, const void *optionalData, u16 lenOfOptionalData)
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
    sServerInitParam init;
    memcpy (&init, userParam, sizeof(sServerInitParam));

    guibridge::Server server;
    server.useLogger(init.logger);
    if (!server.open(2280, init.chServerR, init.chFromServerToRestOfTheWorldW))
        return -1;

    server.run();
    server.close();

    rhea::thread::deleteMsgQ (init.chServerR, init.chServerW);
    rhea::thread::deleteMsgQ (init.chFromServerToRestOfTheWorldR, init.chFromServerToRestOfTheWorldW);
    return 1;
}


//***********************************************************
void guibridge::CmdHandler_selPrices_buildAResponseAndPushItToServer (HThreadMsgW hQMessageToWebserver, u16 handlerID, int numSel, const unsigned int *priceListIN)
{
    //2 byte per handlerID
    //1 byte per indicare il num di selezioni
    //2 byte per la lunghezza della stringa (comprensiva di 0x00 finale)
    //n byte stringa contenenti la lista dei prezzi formattati, separati da §

    const u16 MAX_STR_SIZE = 512;

    char priceList[MAX_STR_SIZE];
    memset (priceList, 0, sizeof(priceList));
    for (int i=0; i<numSel; i++)
    {
        char s[32];
        rhea::string::format::currency (priceListIN[i], 2, '.', s, sizeof(s));

        if (i > 0)
            strcat (priceList, "§");
        strcat (priceList, s);
    }

    const u16 priceListLen = 1 + strlen(priceList);

    u8 data[MAX_STR_SIZE];
    u16 ct = 0;
    data[ct++] = (u8)((handlerID & 0xFF00) >> 8);
    data[ct++] = (u8)(handlerID & 0x00FF);
    data[ct++] = (u8)numSel;
    data[ct++] = (u8)((priceListLen & 0xFF00) >> 8);
    data[ct++] = (u8)(priceListLen & 0x00FF);
    memcpy (&data[ct], priceList, priceListLen);
    ct += priceListLen;

    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_GPU_EVENT, data, ct);
}


/***********************************************************
 * oneBitPerSelectionArray è una sequenza di byte i cui bit sono stati manipolati dalle fn del namespace rhea::bit::
 *
 */
void guibridge::CmdHandler_selAvailability_buildAResponseAndPushItToServer (HThreadMsgW hQMessageToWebserver, u16 handlerID, int numSel, const void *oneBitPerSelectionArray)
{
    //2 byte per handlerID
    //1 byte per indicare il num di selezioni
    //1 bit per ogni selezione
    u8 SIZE = 4 + (numSel/8);
    u8 data[SIZE];

    memset (data, 0, SIZE);
    data[0] = (u8)((handlerID & 0xFF00) >> 8);
    data[1] = (u8)(handlerID & 0x00FF);
    data[2] = (u8)numSel;

    int nByteNeeded = numSel / 8;
    if (nByteNeeded * 8 < numSel)
        nByteNeeded++;
    memcpy (&data[3], oneBitPerSelectionArray, nByteNeeded);

    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_GPU_EVENT, data, nByteNeeded+3);
}


/***********************************************************
 * invia al server lo stato della selezione (in corso, wait...)
 *
 */
void guibridge::CmdHandler_selStatus_buildAResponseAndPushItToServer (HThreadMsgW hQMessageToWebserver, u8 status)
{
    const u16 handlerID = guibridge::eEventType_selectionRequestStatus;

    u8 data[4];
    u8 ct = 0;
    data[ct++] = (u8)((handlerID & 0xFF00) >> 8);
    data[ct++] = (u8)(handlerID & 0x00FF);
    data[ct++] = (u8)status;

    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_GPU_EVENT, data, ct);
}
