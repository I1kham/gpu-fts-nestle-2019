#include "ProtocolWebsocket.h"
#include "../rheaUtils.h"

using namespace rhea;

// WebSocket Universally Unique IDentifier
static const char WS_UUID[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
static const char HEADER_UPGRADE[] = "Upgrade: websocket";
static const char HEADER_CONNECTION[] = "Connection: Upgrade";
static const char HEADER_CONNECTION2[] = "Connection: keep-alive, Upgrade";
static const char HEADER_HOST[] = "Host: ";
static const char HEADER_KEY[] = "Sec-WebSocket-Key: ";
static const char HEADER_VERSION[] = "Sec-WebSocket-Version: ";
static const char HEADER_EXTENSION[] = "Sec-WebSocket-Extensions: ";
static const char HEADER_PROTOCOL[] = "Sec-WebSocket-Protocol: ";


/****************************************************
 * client_sendHandshake
 *
 *	Dovrebbe iniziare l'handshake lato client, ma non lo implemento
 *	per ora perchè non mi serve. 
 */
bool ProtocolWebsocket::handshake_clientSend (IProtocolChannell *ch UNUSED_PARAM, rhea::ISimpleLogger *logger)
{
	if (logger)
	{
		logger->log("handshake..\n");
		logger->incIndent();
	}

	if (logger)
		logger->log("error, function non implemented\n");

	if (logger)
		logger->decIndent();

	return false;
}


//****************************************************
bool ProtocolServer_handshake_check_header (const char *src, const char *header)
{
    return (strncasecmp(src, header, strlen(header)) == 0);
}

//****************************************************
void ProtocolServer_handshake_copy_header (char *out, const char *src, u16 maxSizeOfOutInBytes)
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
bool ProtocolServer_handshake_make_accept (const char *received_key, char *acceptKey, u32 sizeOfAcceptKeyInBytes)
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
    utils::sha1(sha1_key, sizeof(sha1_key), concat_key, strlen(concat_key));

    //converto la sha_key in base 64
    //Mi servono almeno 31 bytes (vedi rhea::base64_howManyBytesNeededForEncoding())
    return utils::base64_encode ((char*)acceptKey, sizeOfAcceptKeyInBytes, sha1_key, sizeof(sha1_key));
}

//****************************************************
bool ProtocolWebsocket::server_isAValidHandshake(const void *bufferIN, u32 sizeOfBuffer)
{
	Handshake hs;
	return priv_server_isAValidHandshake(bufferIN, sizeOfBuffer, &hs);
}

//****************************************************
bool ProtocolWebsocket::priv_server_isAValidHandshake(const void *bufferIN, u32 sizeOfBufferIN, Handshake *out)
{
	const u16 MAX_BUFFER_SIZE = 1024;
	char buffer[MAX_BUFFER_SIZE];

	if (sizeOfBufferIN >= MAX_BUFFER_SIZE - 1)
		return false;
	memcpy(buffer, bufferIN, sizeOfBufferIN);
	buffer[sizeOfBufferIN] = 0;

	memset(out, 0x00, sizeof(Handshake));

	//il buffer di handshake deve iniziare con la classica richiesta HTTP
	if (sscanf(buffer, "GET %s HTTP/1.1", out->resource) != 1)
	{
		//printf ("Invalid HTTP GET request.\n");
		return false;
	}

	//parsing dell'handshake ricevuto
	char *token = NULL;
	char *tokState;
	token = strtok_r(buffer, "\r\n", &tokState);
	while (token)
	{
		if (ProtocolServer_handshake_check_header(token, HEADER_UPGRADE))
			out->upgrade = 1;

		else if (ProtocolServer_handshake_check_header(token, HEADER_CONNECTION))
			out->connection = 1;

		else if (ProtocolServer_handshake_check_header(token, HEADER_CONNECTION2))
			out->connection = 2;

		else if (ProtocolServer_handshake_check_header(token, HEADER_HOST))
			ProtocolServer_handshake_copy_header(out->host, &token[strlen(HEADER_HOST)], sizeof(out->host));

		else if (ProtocolServer_handshake_check_header(token, HEADER_KEY))
			ProtocolServer_handshake_copy_header(out->received_key, &token[strlen(HEADER_KEY)], sizeof(out->received_key));

		else if (ProtocolServer_handshake_check_header(token, HEADER_VERSION))
			out->version = rhea::string::utf8::toU32((const u8*)&token[strlen(HEADER_VERSION)]);
		else if (ProtocolServer_handshake_check_header(token, HEADER_EXTENSION))
			ProtocolServer_handshake_copy_header(out->extension, &token[strlen(HEADER_EXTENSION)], sizeof(out->extension));

		else if (ProtocolServer_handshake_check_header(token, HEADER_PROTOCOL))
			ProtocolServer_handshake_copy_header(out->protocol, &token[strlen(HEADER_PROTOCOL)], sizeof(out->protocol));


		token = strtok_r(NULL, "\r\n", &tokState);
	}

	//se tutto ok, preparo l'handshake di risposta
	if (out->upgrade && out->connection && out->host && out->received_key && out->version)
		return true;

	return false;
}


//****************************************************
bool ProtocolWebsocket::handshake_serverAnswer(IProtocolChannell *ch, rhea::ISimpleLogger *logger UNUSED_PARAM)
{
	Handshake hs;
	if (!priv_server_isAValidHandshake(ch->getReadBuffer(), ch->getNumBytesInReadBuffer(), &hs))
		return false;
	ch->consumeReadBuffer(ch->getNumBytesInReadBuffer());

    //calcolo la key di risposta
    char    send_key[32];
	if (!ProtocolServer_handshake_make_accept(hs.received_key, send_key, sizeof(send_key)))
		return false;

    //printf ("Handshake request from %s:\n\tReceived key: \t%s\n\tSend key: \t%s\n", hs.host, hs.received_key, send_key);

    //fillo il buffer con la risposta da rimandare indietro
    const char *connection = HEADER_CONNECTION;
    if (hs.connection == 2)
    {
        connection = HEADER_CONNECTION2;
    }
    char answer[300];
    sprintf(answer,    "HTTP/1.1 101 Switching Protocols\r\n"\
                        "Upgrade: websocket\r\n"\
                        "%s\r\n"\
                        "Sec-WebSocket-Accept: %s\r\n"\
                        "Sec-WebSocket-Protocol: %s\r\n\r\n",
                        connection, send_key, hs.protocol);
    //printf ("Handshake response: %s\n", out);


	u16 n = ch->write((const u8*)answer, (u16)strlen(answer), 2000);
    if (n == 0 || n >= protocol::RES_ERROR)
        return false;
    return true;
}




/****************************************************
 * vedi IProtocol.h
 */
u16 ProtocolWebsocket::virt_decodeBuffer (IProtocolChannell *ch, const u8 *buffer, u16 nBytesInBuffer, LinearBuffer &out_result, u32 startOffset, u16 *out_nBytesInseritiInOutResult)
{
	*out_nBytesInseritiInOutResult = 0;

	//prova a decodificare i dati che sono nel buffer di lettura per vedere
	//se riesce a tirarci fuori un frame
	sDecodeResult decoded;
	u16 nBytesConsumed = priv_decodeOneMessage (buffer, nBytesInBuffer, &decoded);
	if (nBytesConsumed == 0 || nBytesConsumed >= protocol::RES_ERROR)
		return nBytesConsumed;


	//in decoded c'è un messaggio buono, vediamo di cosa si tratta
	switch (decoded.opcode)
	{
		case eWebSocketOpcode_TEXT:
		case eWebSocketOpcode_BINARY:
			//copio il payload appena ricevuto nel buffer utente
			if (decoded.payloadLen)
			{
				out_result.write (decoded.payload, startOffset, decoded.payloadLen, true);
				*out_nBytesInseritiInOutResult += (u16)decoded.payloadLen;

				if (decoded.isMasked)
				{
					//unmask del payload se necessario
					u8 *p = out_result._getPointer(startOffset);
					for (u16 i = 0; i < decoded.payloadLen; i++)
						p[i] ^= decoded.keys[i % 4];
				}
			}
			return nBytesConsumed;
			break;

		case eWebSocketOpcode_PING:
		{
			//unmask del payload se necessario
			if (decoded.isMasked)
			{
				DBGBREAK;
				/*u8 *p = decoded.payload;
				for (u16 i = 0; i < decoded.payloadLen; i++)
					p[i] ^= decoded.keys[i % 4];*/
			}
			
			//rispondo con pong
			u8 wBuffer[32];
			u16 n = priv_encodeAMessage(true, eWebSocketOpcode_PONG, decoded.payload, decoded.payloadLen, wBuffer, sizeof(wBuffer));
			ch->write(wBuffer, n, 500);
			return nBytesConsumed;
		}
		break;


		case eWebSocketOpcode_PONG:
			//ho ricevuto un pong
			return nBytesConsumed;
			break;

		case eWebSocketOpcode_CLOSE:
		{
			//rispondo a mia volta con close e chiudo
			sendClose(ch);
			return protocol::RES_PROTOCOL_CLOSED;
		}
		break;


		default:
		{
			//messaggio invalido, chiudo il canale
			sendClose(ch);
			return protocol::RES_PROTOCOL_CLOSED;
		}
		break;
	}

	return protocol::RES_PROTOCOL_CLOSED;
}

/****************************************************
 * Prova ad estrarre un valido messaggio websocket dal buffer e ritorna il numero di bytes "consumati" durante il processo.
 * Se non ci sono abbastanza bytes per un valido completo messaggio, ritorna 0 in quanto non consuma alcun bytes. Si suppone che
 * qualcuno all'esterno continuerà ad appendere bytes al buffer fino a quando questo non conterrà un valido messaggio consumabile da
 * questa fn
 */
u16 ProtocolWebsocket::priv_decodeOneMessage(const u8 *buffer, u16 nBytesInBuffer, sDecodeResult *out_result) const
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
        case 0x00:  out_result->opcode = eWebSocketOpcode_CONTINUATION; 
			break;
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
	out_result->isMasked = ((b & 0x80) != 0);

    //payload length
	out_result->payloadLen = (b & 0x7f);


    //la lunghezza del payload potrebbe essere indicata da ulteriori bytes.
    if (out_result->payloadLen == 126)
    {
        if (nBytesInBuffer < 4)
            return 0;
		out_result->payloadLen = buffer[ct++];
		out_result->payloadLen <<= 8;
		out_result->payloadLen += buffer[ct++];
    }
    //If payloadLen == 127, the following 8 bytes interpreted as a 64-bit unsigned integer
    else if (out_result->payloadLen == 127)
    {
        //non lo sto nemmeno ad implementare. Se voglio mandare più di 65K di roba in un messaggio è certamente un errore
        out_result->opcode = eWebSocketOpcode_UNKNOWN;
        return protocol::RES_PROTOCOL_CLOSED;
    }


    //se il messaggio è cifrato, a seguire ci sono 4 bytes con la chiave
    out_result->keys[0] = out_result->keys[1] = out_result->keys[2] = out_result->keys[3] = 0x00;
    if (out_result->isMasked)
    {
        if (nBytesInBuffer < ct+4)
            return 0;
		out_result->keys[0] = buffer[ct++];
		out_result->keys[1] = buffer[ct++];
		out_result->keys[2] = buffer[ct++];
		out_result->keys[3] = buffer[ct++];
    }

    //payload
    if (nBytesInBuffer < ct + out_result->payloadLen)
        return 0;
    out_result->payload = &buffer[ct];
	ct += out_result->payloadLen;

    return ct;
}

/****************************************************
 * vedi IProtocol.h
 */
u16 ProtocolWebsocket::virt_encodeBuffer(const u8 *bufferToEncode, u16 nBytesToEncode, u8 *out_buffer, u16 sizeOfOutBuffer)
{
	return priv_encodeAMessage (true, eWebSocketOpcode_BINARY, bufferToEncode, nBytesToEncode, out_buffer, sizeOfOutBuffer);
}

/****************************************************
 * Prepara un valido messaggio Websocket e lo mette in wBuffer a partire dal byte 0.
 * Ritorna la lunghezza in bytes del messaggio
 */
u16 ProtocolWebsocket::priv_encodeAMessage(bool bFin, eWebSocketOpcode opcode, const void *payloadToSend, u16 payloadLen, u8 *wBuffer, u16 sizeOfOutBuffer)
{
	u32 nBytesRequired = payloadLen + 5;
	if (sizeOfOutBuffer < nBytesRequired)
		return protocol::RES_PROTOCOL_WRITEBUFFER_TOOSMALL;

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
		wBuffer[ct++] = (u8)((payloadLen & 0xff00) >> 8);
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
		assert(ct + payloadLen <= sizeOfOutBuffer);
		memcpy(&wBuffer[ct], payloadToSend, payloadLen);
		ct += payloadLen;
	}

	return ct;
}




//****************************************************
i16 ProtocolWebsocket::writeText (IProtocolChannell *ch, const char *strIN)
{
    if (NULL == strIN)
        return 1;
    size_t n = strlen(strIN);
    if (n < 1)
    {
        return 1;
    }

	u16 nToWrite = priv_encodeAMessage (true, eWebSocketOpcode_TEXT, strIN, (u16)n, bufferW, BUFFERW_CUR_SIZE);
	if (nToWrite)
	{
		ch->write (bufferW, nToWrite, 1000);
		return (i16)nToWrite;
	}
	return -1;
}

//****************************************************
i16 ProtocolWebsocket::writeBuffer (IProtocolChannell *ch, const void *bufferIN, u16 nBytesToWrite)
{
    u16 nToWrite = priv_encodeAMessage(true, eWebSocketOpcode_BINARY, bufferIN, nBytesToWrite, bufferW, BUFFERW_CUR_SIZE);
	if (nToWrite)
	{
		ch->write(bufferW, nToWrite, 1000);
		return (i16)nToWrite;
	}
    return -1;
}

//****************************************************
void ProtocolWebsocket::sendPing (IProtocolChannell *ch)
{
	u8 wBuffer[32];
	u16 n = priv_encodeAMessage(true, eWebSocketOpcode_PING, NULL, 0, wBuffer, sizeof(wBuffer));
	ch->write(wBuffer, n, 1000);
}

//****************************************************
void ProtocolWebsocket::sendClose(IProtocolChannell *ch)
{
	u8 wBuffer[32];
	u16 n = priv_encodeAMessage(true, eWebSocketOpcode_CLOSE, NULL, 0, wBuffer, sizeof(wBuffer));
	ch->write(wBuffer, n, 200);
}
