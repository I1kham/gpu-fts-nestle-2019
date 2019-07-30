#include "GUIProtocol.h"

using namespace rhea;


//****************************************************
GUIProtocol::GUIProtocol (rhea::Allocator *allocatorIN, u32 sizeOfWriteBuffer)
{
    allocator = allocatorIN;
    nBytesInReadBuffer = 0;

    readBufferSize = 1024;
    rBuffer = (u8*)allocator->alloc(readBufferSize, __alignof(u8*));
    wBuffer = (u8*)allocator->alloc(sizeOfWriteBuffer, __alignof(u8*));
}

//****************************************************
GUIProtocol::~GUIProtocol()
{
    allocator->dealloc(rBuffer);
    allocator->dealloc(wBuffer);
}

//****************************************************
void GUIProtocol::close(OSSocket &sok)
{
    if (OSSocket_isOpen(sok))
    {
        sendClose(sok);
        OSSocket_close(sok);
    }

    nBytesInReadBuffer = 0;
}

//****************************************************
void GUIProtocol::priv_growReadBuffer()
{
    u32 newReadBufferSize = readBufferSize + 1024;
    if (newReadBufferSize < 0xffff)
    {
        u8 *p = (u8*)allocator->alloc(newReadBufferSize);
        memcpy (p, rBuffer, readBufferSize);
        readBufferSize = (u16)newReadBufferSize;

        allocator->dealloc(rBuffer);
        rBuffer= p;
    }
}

/****************************************************
 * Legge dalla socket ed eventualmente interpreta i messaggi ricevuti
 * I messaggi di stato sono gestiti automaticamente qui dentro (ping, pong, close..)
 * Nel caso si ricevano messaggi TEXT o BINARY, allora il loro payload viene messo in out_buffer
 *
 * Ritorna il numero di bytes messi in [out_buffer].
 */
u16 GUIProtocol::read (OSSocket &sok, rhea::LinearBuffer *out_buffer, u8 *bSocketWasClosed)
{
    *bSocketWasClosed = 0;

    u32 nMaxToRead = readBufferSize - nBytesInReadBuffer;
    if (nMaxToRead == 0)
    {
        priv_growReadBuffer();
        nMaxToRead = readBufferSize - nBytesInReadBuffer;
    }

    i32 nBytesLetti = OSSocket_read (sok, &rBuffer[nBytesInReadBuffer], nMaxToRead, 0);
    if (nBytesLetti == 0)
    {
        *bSocketWasClosed = 1;
        return 0;
    }

    if (nBytesLetti < 0)
        return 0;

    nBytesInReadBuffer += (u16)nBytesLetti;

    u16 bytesAppendedToOutBuffer = 0;
    while (nBytesInReadBuffer)
    {
        //prova a decodificare i dati che sono nel buffer di lettura per vedere
        //se riesce a tirarci fuori un frame
        sDecodeResult decoded;
        u16 nBytesConsumati = priv_decodeBuffer (rBuffer, nBytesInReadBuffer, &decoded);
        if (nBytesConsumati == u16MAX)
        {
            *bSocketWasClosed = 1;
            OSSocket_close(sok);
            return bytesAppendedToOutBuffer;
        }

        if (nBytesConsumati == 0)
            return bytesAppendedToOutBuffer;

        //in decoded c'è un messaggio buono, vediamo di cosa si tratta
        switch (decoded.opcode)
        {
        case eWebSocketOpcode_TEXT:
        case eWebSocketOpcode_BINARY:
            //copio il payload appena ricevuto nel buffer utente
            if (decoded.payloadLenInBytes)
            {
                out_buffer->write (decoded.payload, 0, decoded.payloadLenInBytes, true);
                bytesAppendedToOutBuffer += (u16)decoded.payloadLenInBytes;
            }
            break;

        case eWebSocketOpcode_CLOSE:
            {
                //rispondo con close e chiudo
                u16 n = priv_encodeBuffer (true, eWebSocketOpcode_CLOSE, NULL, 0);
                priv_sendWBuffer(sok, n);

                OSSocket_close(sok);
                *bSocketWasClosed = 1;
                return bytesAppendedToOutBuffer;
            }
            break;

        case eWebSocketOpcode_PING:
            {
                printf ("PING\n");
                //rispondo con pong
                u16 n = priv_encodeBuffer (true, eWebSocketOpcode_PONG, decoded.payload, decoded.payloadLenInBytes);
                priv_sendWBuffer(sok, n);
            }
            break;


        case eWebSocketOpcode_PONG:
            printf("PONG\n");
            break;

        default:
            {
                printf ("???\n");
                //ho ricevuto un OPCODE non supportato, chiudo
                u16 n = priv_encodeBuffer (true, eWebSocketOpcode_CLOSE, NULL, 0);
                priv_sendWBuffer(sok, n);

                OSSocket_close(sok);
                 *bSocketWasClosed = 1;
                return bytesAppendedToOutBuffer;
            }
            break;
        }


        //shifto il read buffer
        assert(nBytesInReadBuffer >= nBytesConsumati);
        nBytesInReadBuffer -= nBytesConsumati;
        if (nBytesInReadBuffer >  0)
            memcpy (rBuffer, &rBuffer[nBytesConsumati], nBytesInReadBuffer);
    }

    return bytesAppendedToOutBuffer;
}

/****************************************************
 * Prova ad estrarre un valido messaggio websocket dal buffer e ritorna il numero di bytes "consumati" durante il processo.
 * Se non ci sono abbastanza bytes per un valido completo messaggio, ritorna 0 in quanto non consuma alcun bytes. Si suppone che
 * qualcuno all'esterno continuerà ad appendere bytes al buffer fino a quando questo non conterrà un valido messaggio consumabile da
 * questa fn
 */
u16 GUIProtocol::priv_decodeBuffer (u8 *buffer, u16 nBytesInBuffer, sDecodeResult *out_result) const
{
    //ci devono essere almeno 2 bytes, questi sono obbligatori, il protocollo Websocket non ammette msg lunghi meno di 2 bytes
    if (nBytesInBuffer < 2)
        return 0;

    u16 ct=0;

    //dal primo byte si ricava fin, RSV1, RSV2, RSV3, opcode
    u8 b = buffer[ct++];
    {
        out_result->bIsLastFrame = ((b & 0x80) != 0);

        //MUST be 0 unless an extension is negotiated that defines meanings for non-zero values.
        //u8 RSV1 = (b & 0x40);
        //u8 RSV2 = (b & 0x20);
        //u8 RSV3 = (b & 0x10);

        //opcode
        switch ((b & 0x0F))
        {
        case 0x00:  out_result->opcode = eWebSocketOpcode_CONTINUATION; break;
        case 0x01:  out_result->opcode = eWebSocketOpcode_TEXT; break;
        case 0x02:  out_result->opcode = eWebSocketOpcode_BINARY; break;
        case 0x08:  out_result->opcode = eWebSocketOpcode_CLOSE; break;
        case 0x09:  out_result->opcode = eWebSocketOpcode_PING; break;
        case 0x0A:  out_result->opcode = eWebSocketOpcode_PONG; break;

        default:    out_result->opcode = eWebSocketOpcode_UNKNOWN; break;
        }
    }

    //dal secondo byte si ricava "isMasked" e una indicazione sulla lunghezza del payload
    b = buffer[ct++];
        bool isMasked = ((b & 0x80) != 0);

        //payload length
        u16 payloadLen = (b & 0x7f);


    //la lunghezza del payload potrebbe essere indicata da ulteriori bytes.
    if (payloadLen == 126)
    {
        if (nBytesInBuffer < 4)
            return 0;
        payloadLen = buffer[ct++];
        payloadLen <<= 8;
        payloadLen += buffer[ct++];
    }
    //If payloadLen == 127, the following 8 bytes interpreted as a 64-bit unsigned integer
    else if (payloadLen == 127)
    {
        //non lo sto nemmeno ad implementare. Se voglio mandare più di 65K di roba in un messaggio è certamente un errore
        out_result->opcode = eWebSocketOpcode_UNKNOWN;
        return u16MAX;
    }
    out_result->payloadLenInBytes = payloadLen;


    //se il messaggio è cifrato, a seguire ci sono 4 bytes con la chiave
    u8 keys[4] = {0,0,0,0};
    if (isMasked)
    {
        if (nBytesInBuffer < ct+4)
            return 0;
        keys[0] = buffer[ct++];
        keys[1] = buffer[ct++];
        keys[2] = buffer[ct++];
        keys[3] = buffer[ct++];
    }

    //payload
    if (nBytesInBuffer < ct + payloadLen)
        return 0;
    out_result->payload = &buffer[ct];

    if(isMasked)
    {
        //unmask del payload se necessario
        for(u16 i=0; i<payloadLen; i++)
            buffer[ct++] ^= keys[i % 4];
    }
    else
        ct += payloadLen;

    return ct;
}

//****************************************************
i16 GUIProtocol::writeText (OSSocket &sok, const char *strIN)
{
    if (NULL == strIN)
        return 1;
    size_t n = strlen(strIN);
    if (n < 1)
        return 1;

    u16 nToWrite = priv_encodeBuffer (true, eWebSocketOpcode_TEXT, strIN, n);
    if (nToWrite)
        return priv_sendWBuffer(sok, nToWrite);
    return -1;
}

//****************************************************
i16 GUIProtocol::writeBuffer(OSSocket &sok, const void *bufferIN, u16 nBytesToWrite)
{
    u16 nToWrite = priv_encodeBuffer (true, eWebSocketOpcode_BINARY, bufferIN, nBytesToWrite);

    if (nToWrite)
        return priv_sendWBuffer(sok, nToWrite);
    return -1;
}

//****************************************************
void GUIProtocol::sendPing(OSSocket &sok)
{
    u16 n = priv_encodeBuffer (true, eWebSocketOpcode_PING, NULL, 0);
    priv_sendWBuffer(sok, n);
}

//****************************************************
void GUIProtocol::sendClose(OSSocket &sok)
{
    u16 n = priv_encodeBuffer (true, eWebSocketOpcode_CLOSE, NULL, 0);
    priv_sendWBuffer(sok, n);
}

//****************************************************
i16 GUIProtocol::priv_sendWBuffer (OSSocket &sok, u16 nBytesToWrite)
{
    i32 nWrittenSoFar = 0;
    while (1)
    {
        i32 n = OSSocket_write (sok, &wBuffer[nWrittenSoFar], (nBytesToWrite-nWrittenSoFar) );
        if (n == 0)
            return 0;
        if (n > 0)
        {
            nWrittenSoFar += n;
            if (nWrittenSoFar >= nBytesToWrite)
                return 1;
        }

        OS_sleepMSec(50);
    }
}

/****************************************************
 * Prepara un valido messaggio Websocket e lo mette in wBuffer a partire dal byte 0.
 * Ritorna la lunghezza in bytes del messaggio
 */
u16 GUIProtocol::priv_encodeBuffer (bool bFin, eWebSocketOpcode opcode, const void *payloadToSend, u16 payloadLen)
{
    u16 ct = 0;

    //primo byte : fin, RSV1, RSV2, RSV3, opcode
    wBuffer[ct] = (u8)opcode;
    if (bFin)
        wBuffer[ct] |= 0x80;
    ++ct;

    //secondo byte: isMasked | payloadLen
    if (payloadLen < 126)
        wBuffer[ct++] = (u8)payloadLen;
    else if (payloadLen < 0xffff)
    {
        wBuffer[ct++] = 126;
        wBuffer[ct++] = (u8)((payloadLen & 0xff00) >>8 );
        wBuffer[ct++] = (u8)(payloadLen & 0x00ff);
    }
    else
    {
        //i messaggi a lunghezza superiore ai 65k non li supporto
        return 0;
    }

    //payload
    if (payloadLen > 0)
    {
        memcpy(&wBuffer[ct], payloadToSend, payloadLen);
        ct += payloadLen;
    }

    return ct;
}
