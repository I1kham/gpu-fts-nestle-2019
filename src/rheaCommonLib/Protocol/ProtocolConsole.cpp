#include "ProtocolConsole.h"
#include "../OS/OS.h"
#include "../rheaUtils.h"
#include "../rheaRandom.h"
#include <stdio.h>
#include <time.h>

using namespace rhea;

/****************************************************
 * client_sendHandshake
 *
 * un client connesso ad un server deve mandare questo messaggio
 * per farsi riconoscere come console
 */
bool ProtocolConsole::handshake_clientSend(IProtocolChannell *ch, rhea::ISimpleLogger *logger)
{
    if (logger)
    {
        logger->log ("handshake..\n");
        logger->incIndent();
    }

    rhea::Random random((u32)time(NULL));
    const u8 key = random.getU32(255);

    char handshake[32];
    sprintf (handshake, "RHEACONSOLE");
    handshake[11] = key;

    if (logger)
        logger->log ("sending with key=%d\n", key);
    ch->write ((const u8*)handshake, 12, 2000);


	//aspetta la risposta dal server
	u8 bChannelWasClosed = 0;
    u16 n = ch->read (5000);
	if (n >= protocol::RES_ERROR)
	{
		if (logger)
			logger->log("error while waiting. Error code is [%d]r\n", n);
		return false;
	}

    if (logger)
        logger->log ("response length is %d bytes\n", ch->getNumBytesInReadBuffer());
    if (ch->getNumBytesInReadBuffer() < 12)
    {
		ch->consumeReadBuffer(ch->getNumBytesInReadBuffer());
        if (logger)
        {
            logger->log("FAIL\n");
            logger->decIndent();
        }
        return false;
    }

	const u8 *p = ch->getReadBuffer();
    const u8 expectedKey = 0xff - key;
    if (memcmp (p, "RHEACONSOLE", 11) != 0 || (u8)p[11] != expectedKey)
    {
        if (logger)
        {
			memcpy(handshake, p, 12);
            u8 receivedKey = handshake[11];
            handshake[11] = 0;
            logger->log("Invalid answer: [%s], received key=%d, expected=%d\n", handshake, receivedKey, expectedKey);
            logger->decIndent();
			ch->consumeReadBuffer(12);
        }
        return false;
    }
	

	ch->consumeReadBuffer(12);
	if (logger)
    {
        logger->log("Done!\n");
        logger->decIndent();
    }
    return true;
}


//****************************************************
bool ProtocolConsole::server_isAValidHandshake (const void *bufferIN, u32 sizeOfBuffer)
{
    if (sizeOfBuffer < 12)
        return false;

    if (memcmp (bufferIN, "RHEACONSOLE", 11) != 0)
        return false;

	return true;

}

//****************************************************
bool ProtocolConsole::handshake_serverAnswer(IProtocolChannell *ch, rhea::ISimpleLogger *logger)
{
	if (!server_isAValidHandshake(ch->getReadBuffer(), ch->getNumBytesInReadBuffer()))
		return false;

	const u8 *buffer = ch->getReadBuffer();
	const u8 key = 0xff - buffer[11];
	ch->consumeReadBuffer(12);

	char answer[16];
	sprintf(answer, "RHEACONSOLE");
	answer[11] = key;
	u16 n = ch->write ((const u8*)answer, 12, 3000);

	if (n != 12)
		return false;
	return true;
}



/****************************************************
 * vedi IProtocol.h
 */
u16 ProtocolConsole::virt_decodeBuffer (IProtocolChannell *ch, const u8 *buffer, u16 nBytesInBuffer, LinearBuffer &out_result, u16 *out_nBytesInseritiInOutResult)
{
	*out_nBytesInseritiInOutResult = 0;

	//prova a decodificare i dati che sono nel buffer di lettura per vedere
	//se riesce a tirarci fuori un frame
	sDecodeResult decoded;
	u16 nBytesConsumed = priv_decodeOneMessage (buffer, nBytesInBuffer, &decoded);
	if (nBytesConsumed == 0 || nBytesConsumed >= protocol::RES_ERROR)
		return nBytesConsumed;


	//in decoded c'è un messaggio buono, vediamo di cosa si tratta
	switch (decoded.what)
	{
		case eOpcode_msg:
			//copio il payload appena ricevuto nel buffer utente
			if (decoded.payloadLen)
			{
				out_result.write(decoded.payload, 0, decoded.payloadLen, true);
				*out_nBytesInseritiInOutResult += (u16)decoded.payloadLen;
			}
			return nBytesConsumed;
			break;

		case eOpcode_close:
		{
			//rispondoa a mia volta con close e chiudo
			virt_sendCloseMessage(ch);
			return protocol::RES_PROTOCOL_CLOSED;
		}
		break;

		case eOpcode_unknown:
			return nBytesConsumed;

		default:
		{
			//messaggio invalido, chiudo il protocollo
			virt_sendCloseMessage(ch);
			return protocol::RES_PROTOCOL_CLOSED;
		}
		break;
	}

	return u16MAX;
}

/****************************************************
 * Prova ad estrarre un valido messaggio dal buffer e ritorna il numero di bytes "consumati" durante il processo.
 * Se non ci sono abbastanza bytes per un valido completo messaggio, ritorna 0 in quanto non consuma alcun bytes. Si suppone che
 * qualcuno all'esterno continuerà ad appendere bytes al buffer fino a quando questo non conterrà un valido messaggio consumabile da
 * questa fn
 */
u16 ProtocolConsole::priv_decodeOneMessage(const u8 *buffer, u16 nBytesInBuffer, sDecodeResult *out_result) const
{
    //ci devono essere almeno 3 bytes, questi sono obbligatori, il protocollo non ammette msg lunghi meno di 3 bytes
    if (nBytesInBuffer < 3)
        return 0;
	
    //i primi 2 byte sono il magic code
	u16 ct = 0;
	if (buffer[ct++] != MAGIC_CODE_1)
        return 1;
    if (buffer[ct++] != MAGIC_CODE_2)
        return 2;

    //il terzo byte è l'opcode
	out_result->what = (eOpcode)buffer[ct++];
	switch (out_result->what)
	{
		default:
			//Errore, opcode non riconosciuto
			printf("ERR ProtocolConsole::priv_decodeOneMessage() => invalid opcode [%d]\n", out_result->what);
			out_result->what = eOpcode_unknown;
			out_result->payloadLen = 0;
			return 3;

		case eOpcode_close:
			//ho ricevuto una esplicita richiesta di chiudere il canale
			return protocol::RES_PROTOCOL_CLOSED;
	
		case eOpcode_internal_simpledMsg:
			{
				/*  messaggio semplice, payloadLen <=255 byte, ck8 semplice a fine messaggio:
						MAGIC1 MAGIC2 WHAT LEN data data data CK
				*/
				out_result->what = eOpcode_msg;
				out_result->payloadLen = buffer[ct++];

				//mi servono in tutto 4 + payloadLen +1 bytes nel buffer
				if (nBytesInBuffer < 5 + out_result->payloadLen)
					return 0;
				out_result->payload = &buffer[ct];
				ct += out_result->payloadLen;

				//verifichiamo CK
				const u8 ck = buffer[ct++];
				const u8 calc_ck = rhea::utils::simpleChecksum8_calc (out_result->payload, out_result->payloadLen);
				if (ck == calc_ck)
					return ct;

				printf("ERR ProtocolConsole::decodeBuffer() => bad CK [%d] expected [%d]\n", ck, calc_ck);
				out_result->what = eOpcode_unknown;
				return ct;
			}
			break;

		case eOpcode_internal_extendedMsg:
			{
				//MAGIC1 MAGIC2 WHAT LEN_MSB LEN_LSB payload...payload CK(u16)
				out_result->what = eOpcode_msg;
				out_result->payloadLen = buffer[ct++];
				out_result->payloadLen <<= 8;
				out_result->payloadLen |= buffer[ct++];

				//mi servono in tutto 5 + payloadLen +2 bytes nel buffer
				if (nBytesInBuffer < 7 + out_result->payloadLen)
					return 0;
				out_result->payload = &buffer[ct];
				ct += out_result->payloadLen;

				//verifichiamo CK
				u16 ck = buffer[ct++];
				ck <<= 8;
				ck |= buffer[ct++];

				const u16 calc_ck = rhea::utils::simpleChecksum16_calc(out_result->payload, out_result->payloadLen);
				if (ck == calc_ck)
					return ct;

				printf("ERR ProtocolConsole::decodeBuffer() => bad CK [%d] expected [%d]\n", ck, calc_ck);
				out_result->what = eOpcode_unknown;
				return ct;

			}
			break;
	}
}



/****************************************************
 * vedi IProtocol.h
 */
u16 ProtocolConsole::virt_encodeBuffer(const u8 *bufferToEncode, u16 nBytesToEncode, u8 *out_buffer, u16 sizeOfOutBuffer)
{
	return priv_encodeAMessage(eOpcode_msg, bufferToEncode, nBytesToEncode, out_buffer, sizeOfOutBuffer);
}

/****************************************************
 * Prepara un valido messaggio e lo mette in wBuffer a partire dal byte 0.
 * Ritorna la lunghezza in bytes del messaggio
 */
u16 ProtocolConsole::priv_encodeAMessage (eOpcode opcode, const void *payloadToSend, u16 payloadLen, u8 *wBuffer, u16 sizeOfOutBuffer) const
{
	if (sizeOfOutBuffer < 3)
	{
		DBGBREAK;
		return 0;
	}

	if (opcode == eOpcode_msg)
	{
		if (payloadLen == 0)
			return 0;
		if (payloadLen <= 0xff)
			opcode = eOpcode_internal_simpledMsg;
		else
			opcode = eOpcode_internal_extendedMsg;
	}

    u16 ct = 0;
	wBuffer[ct++] = MAGIC_CODE_1;
	wBuffer[ct++] = MAGIC_CODE_2;
	wBuffer[ct++] = (u8)opcode;

	switch (opcode)
	{
		default:
			DBGBREAK;
			return 0;

		case eOpcode_close:
			return ct;

		case eOpcode_internal_simpledMsg:
			//MAGIC1 MAGIC2 WHAT LEN(byte) payload...payload CK(byte)
			if (sizeOfOutBuffer < 5 + payloadLen)
			{
				DBGBREAK;
				return 0;
			}

			wBuffer[ct++] = (u8)payloadLen;
			memcpy (&wBuffer[ct], payloadToSend, payloadLen);
			ct += payloadLen;
			wBuffer[ct++] = rhea::utils::simpleChecksum8_calc (payloadToSend, payloadLen);
			return ct;
    

		case eOpcode_internal_extendedMsg:
			if (payloadLen == 0)
				return 0;

			//MAGIC1 MAGIC2 WHAT LEN_MSB LEN_LSB payload...payload CK(u16)
			if (sizeOfOutBuffer < 7 + payloadLen)
			{
				DBGBREAK;
				return 0;
			}

			wBuffer[ct++] = (u8)((payloadLen & 0xFF00) >> 8);
			wBuffer[ct++] = (u8) (payloadLen & 0x00FF);
			
			memcpy(&wBuffer[ct], payloadToSend, payloadLen);
			ct += payloadLen;

			u16 ck = rhea::utils::simpleChecksum16_calc (payloadToSend, payloadLen);
			wBuffer[ct++] = (u8)((ck & 0xFF00) >> 8);
			wBuffer[ct++] = (u8) (ck & 0x00FF);
			return ct;
	}
}



//****************************************************
void ProtocolConsole::virt_sendCloseMessage(IProtocolChannell *ch)
{
	u8 wBuffer[32];
	u16 n = priv_encodeAMessage (eOpcode_close, NULL, 0, wBuffer, sizeof(wBuffer));
	ch->write (wBuffer, n, 1000);
}


