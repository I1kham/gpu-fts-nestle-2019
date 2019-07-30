#include "ConsoleProtocol.h"
#include "OS/OS.h"
#include "rheaUtils.h"
#include "rheaRandom.h"
#include <stdio.h>

using namespace rhea;

//****************************************************
ConsoleProtocol::ConsoleProtocol(rhea::Allocator *allocatorIN, u32 sizeOfWriteBuffer)
{
    allocator = allocatorIN;
    nBytesInReadBuffer = 0;

    readBufferSize = 1024;
    rBuffer = (u8*)allocator->alloc(readBufferSize, __alignof(u8*));
    wBuffer = (u8*)allocator->alloc(sizeOfWriteBuffer, __alignof(u8*));
}

//****************************************************
ConsoleProtocol::~ConsoleProtocol()
{
    allocator->dealloc(rBuffer);
    allocator->dealloc(wBuffer);
}

/****************************************************
 * client_handshake
 *
 * un client connesso ad un server deve mandare questo messaggio
 * per farsi riconoscere come console
 */
bool ConsoleProtocol::client_handshake (OSSocket &sok)
{
    rhea::Random random(time(NULL));
    const u8 key = random.getU32(255);

    char handshake[32];
    sprintf (handshake, "RHEACONSOLE");
    handshake[11] = key;
    OSSocket_write (sok, handshake, 12);

    i16 n = OSSocket_read (sok, handshake, sizeof(handshake), 5000);
    if (n < 12)
        return false;

    const u8 expectedKey = 0xff - key;
    if (memcmp (handshake, "RHEACONSOLE", 11) != 0 || handshake[11] != expectedKey)
        return false;

    return true;
}


/****************************************************
 * server_isValidaHandshake
 *
 * il server riceve come primo messaggio dalla socket un handshake
 * Se questo corrisponde a quello "Buono", ritorna il numero di bytes consumati,
 * altrimenti ritorna 0
 */
i16 ConsoleProtocol::server_isValidaHandshake (const void *bufferIN, u32 sizeOfBuffer, OSSocket &sok)
{
    if (sizeOfBuffer < 12)
        return 0;

    if (memcmp (bufferIN, "RHEACONSOLE", 11) != 0)
        return 0;

    const u8 *buffer = (const u8*)bufferIN;
    const u8 key = 0xff - buffer[11];
    char answer[16];
    sprintf (answer, "RHEACONSOLE");
    answer[11] = key;
    OSSocket_write (sok, answer, 12);

    return 12;
}



//****************************************************
void ConsoleProtocol::close (OSSocket &sok)
{
    if (OSSocket_isOpen(sok))
    {
        sendClose(sok);
        OSSocket_close(sok);
    }
    nBytesInReadBuffer = 0;
}

//****************************************************
void ConsoleProtocol::priv_growReadBuffer()
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
 * Eventuali messaggi di stato sono gestiti automaticamente qui dentro (ping, pong, close..)
 * Nel caso si ricevano messaggi utili (es eOpcode_simpleMsg), allora il loro payload viene messo in out_buffer
 *
 * Ritorna il numero di bytes messi in [out_buffer].
 */
u16 ConsoleProtocol::read (OSSocket &sok, rhea::LinearBuffer *out_buffer, u8 *bSocketWasClosed)
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
        switch (decoded.what)
        {
        case eOpcode_simpleMsg:
            //copio il payload appena ricevuto nel buffer utente
            if (decoded.payloadLenInBytes)
            {
                out_buffer->write (decoded.payload, 0, decoded.payloadLenInBytes, true);
                bytesAppendedToOutBuffer += (u16)decoded.payloadLenInBytes;
            }
            break;

        case eOpcode_close:
            {
                //rispondo con close e chiudo
                u16 n = priv_encodeBuffer (eOpcode_close, NULL, 0);
                priv_sendWBuffer(sok, n);

                OSSocket_close(sok);
                *bSocketWasClosed = 1;
                return bytesAppendedToOutBuffer;
            }
            break;

        case eOpcode_ping:
            {
                printf ("PING\n");
                //rispondo con pong
                u16 n = priv_encodeBuffer (eOpcode_pong, NULL, 0);
                priv_sendWBuffer(sok, n);
            }
            break;

        case eOpcode_pong:
            printf("PONG\n");
            break;

        default:
            //ho ricevuto un OPCODE non supportato, ignoro
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
 * Prova ad estrarre un valido messaggio dal buffer e ritorna il numero di bytes "consumati" durante il processo.
 * Se non ci sono abbastanza bytes per un valido completo messaggio, ritorna 0 in quanto non consuma alcun bytes. Si suppone che
 * qualcuno all'esterno continuerà ad appendere bytes al buffer fino a quando questo non conterrà un valido messaggio consumabile da
 * questa fn
 */
u16 ConsoleProtocol::priv_decodeBuffer (u8 *buffer, u16 nBytesInBuffer, sDecodeResult *out_result) const
{
    //ci devono essere almeno 3 bytes, questi sono obbligatori, il protocollo non ammette msg lunghi meno di 3 bytes
    if (nBytesInBuffer < 3)
        return 0;

    u16 ct=0;

    out_result->what = eOpcode_unknown;
    out_result->payloadLenInBytes =0;
    out_result->payload = NULL;

    //i primi 2 byte sono il magic code
    if (buffer[ct++] != MAGIC_CODE_1)
        return 1;
    if (buffer[ct++] != MAGIC_CODE_2)
        return 2;

    //il terzo byte è l'opcode
    const eOpcode what = (eOpcode)buffer[ct++];
    if (what == eOpcode_simpleMsg)
    {
        /*  messaggio semplice, payloadLen <=255 byte, ck semplice a fine messaggio
                MAGIC1 MAGIC2 WHAT LEN data data data CK
        */
        out_result->what = eOpcode_simpleMsg;
        out_result->payloadLenInBytes = buffer[ct++];

        //mi servono in tutto 4 + payloadLen +1 bytes nel buffer
        if (nBytesInBuffer < 5+out_result->payloadLenInBytes)
            return 0;
        out_result->payload = &buffer[ct];
        ct += out_result->payloadLenInBytes;

        //verifichiamo CK
        const u8 ck = buffer[ct++];
        const u8 calc_ck = rhea::utils::simpleChecksum8_calc (out_result->payload, out_result->payloadLenInBytes);
        if (ck == calc_ck)
            return ct;

        printf ("ERR ConsoleProtocol::priv_decodeBuffer() => bad CK [%d] expected [%d]\n", ck, calc_ck);
        out_result->what = eOpcode_unknown;
        return ct;
    }
    else if (what == eOpcode_ping || what == eOpcode_pong || what == eOpcode_close)
    {
        out_result->what = what;

        //verifichiamo CK
        const u8 ck = buffer[ct++];
        const u8 calc_ck = rhea::utils::simpleChecksum8_calc (buffer, 4);
        if (ck == calc_ck)
            return ct;

        printf ("ERR ConsoleProtocol::priv_decodeBuffer() => bad CK [%d] expected [%d]\n", ck, calc_ck);
        out_result->what = eOpcode_unknown;
        return ct;
    }


    //Errore, opcode non riconosciuto
    printf ("ERR ConsoleProtocol::priv_decodeBuffer() => invalid opcode [%d]\n", what);
    out_result->what = eOpcode_unknown;
    return 3;
}

/****************************************************
 * Prepara un valido messaggio e lo mette in wBuffer a partire dal byte 0.
 * Ritorna la lunghezza in bytes del messaggio
 */
u16 ConsoleProtocol::priv_encodeBuffer (eOpcode opcode, const void *payloadToSend, u16 payloadLen)
{
    if (payloadLen == 0)
        return 0;

    u16 ct = 0;
    wBuffer[ct++] = MAGIC_CODE_1;
    wBuffer[ct++] = MAGIC_CODE_2;
    wBuffer[ct++] = (u8)opcode;

    if (opcode == eOpcode_simpleMsg)
    {
        assert (payloadLen <= 0xff);
        wBuffer[ct++] = (u8)payloadLen;
        memcpy (&wBuffer[ct], payloadToSend, payloadLen);
        ct += payloadLen;
        wBuffer[ct++] = rhea::utils::simpleChecksum8_calc (payloadToSend, payloadLen);
        return ct;
    }
    else if (opcode == eOpcode_ping || opcode == eOpcode_pong || opcode == eOpcode_close)
    {
        wBuffer[ct++] = rhea::utils::simpleChecksum8_calc (payloadToSend, payloadLen);
        return ct;
    }

    DBGBREAK;
    return 0;
}

//****************************************************
i16 ConsoleProtocol::writeBuffer (OSSocket &sok, const void *bufferIN, u16 nBytesToWrite)
{
    assert (nBytesToWrite<=0xff);
    u16 nToWrite = priv_encodeBuffer (eOpcode_simpleMsg, bufferIN, nBytesToWrite);
    if (nToWrite)
        return priv_sendWBuffer(sok, nToWrite);
    return -1;
}

//****************************************************
void ConsoleProtocol::sendPing(OSSocket &sok)
{
    u16 n = priv_encodeBuffer (eOpcode_ping, NULL, 0);
    priv_sendWBuffer(sok, n);
}

//****************************************************
void ConsoleProtocol::sendClose(OSSocket &sok)
{
    u16 n = priv_encodeBuffer (eOpcode_close, NULL, 0);
    priv_sendWBuffer(sok, n);
}

//****************************************************
i16 ConsoleProtocol::priv_sendWBuffer (OSSocket &sok, u16 nBytesToWrite) const
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

