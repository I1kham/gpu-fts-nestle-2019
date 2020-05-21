#include "socketListenerCore.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "CmdHandler_ajaxReq.h"
#include "CmdHandler_eventReq.h"

using namespace rasPI::socketListener;


//********************************************
Core::Core()
{
    localAllocator = NULL;
    logger = &nullLogger;
    server = NULL;
	eventSeqNumber = 0;
}


//***************************************************
void Core::useLogger (rhea::ISimpleLogger *loggerIN)
{
    if (NULL==loggerIN)
        logger = &nullLogger;
    else
        logger = loggerIN;
}


//********************************************
bool Core::open (u16 tcpPortNumber)
{
    logger->log ("socketListenerCore::open [port=%d]\n", tcpPortNumber);
    logger->incIndent();

    localAllocator = RHEANEW(rhea::getSysHeapAllocator(), rhea::AllocatorSimpleWithMemTrack) ("sokListenerCore");

    server = RHEANEW(localAllocator, rhea::ProtocolSocketServer)(8, localAllocator);
    server->useLogger(logger);

	//apro la socket TCP
	logger->log("opening TCP socket...");
    eSocketError err = server->start (tcpPortNumber);
    if (err != eSocketError_none)
    {
        logger->log("FAIL\n");
        logger->decIndent();
        RHEADELETE(localAllocator, server);
        return false;
    }
    logger->log("OK\n");


    //buffer di invio
	SEND_BUFFER_SIZE = 4096;
	sendBuffer = (u8*)RHEAALLOC(localAllocator, SEND_BUFFER_SIZE);

    //linear buffer per la ricezione di msg dai client
    buffer.setup (localAllocator, 256);

	//elenco dei clienti identificati
    identifiedClientList.setup (localAllocator);

    //lista degli handler dei messaggi
    cmdHandlerList.setup (localAllocator);

	_nextHandlerID = 0;

    return true;
}

//********************************************
void Core::run()
{
    bool bQuit = false;
	u64 nextTimePurgeCmdHandlerList = 0;

    while (bQuit == false)
    {
        //la wait() si sveglia se è in arrivo una nuova connessione, oppure se un client già  connesso invia dati
        const u8 nEvents = server->wait (5000);
        const u64 timeNowMSec = rhea::getTimeNowMSec();

        //elimino eventuali handler rimasti pendenti
        if (timeNowMSec > nextTimePurgeCmdHandlerList)
        {
            cmdHandlerList.purge (localAllocator, timeNowMSec);
			identifiedClientList.purge(timeNowMSec);
			nextTimePurgeCmdHandlerList = timeNowMSec + 10000;
        }
    
		//analizzo gli eventi ricevuti
        for (u8 i=0; i<nEvents; i++)
        {
            switch (server->getEventType(i))
            {
            default:
                logger->log ("socketListenerCore::run(), unknown event type: %d\n", server->getEventType(i));
                break;

			case rhea::ProtocolSocketServer::evt_ignore:
				break;

            case rhea::ProtocolSocketServer::evt_new_client_connected:
                {
                    //un client vuole connettersi. Accetto la socket in ingresso, ma mi riservo di chiudere la
                    //connessione se il prossimo msg che manda è != dal messaggio di identificazione.
                }
                break;

            case rhea::ProtocolSocketServer::evt_client_has_data_avail:
				//uno dei clienti già  connessi ha mandato qualcosa
                priv_onClientHasDataAvail (i, timeNowMSec);
                break;

            case rhea::ProtocolSocketServer::evt_osevent_fired:
                //uno degli OSEvent associati alla waitList è stato fired.
                //Al momento non dovrebbe mai succedere dato che non ci sono OSEvent da monitorare
                break;
            }
        }
    }


    //free delle risorse
	cmdHandlerList.purge (localAllocator, u64MAX);
    server->close();
    RHEADELETE(localAllocator, server);
    RHEAFREE(localAllocator, sendBuffer);
    buffer.unsetup();
	cmdHandlerList.unsetup();
	identifiedClientList.unsetup ();
    RHEADELETE(rhea::getSysHeapAllocator(), localAllocator);
}

//*****************************************************************
bool Core::priv_extractOneMessage(u8 *bufferPt, u16 nBytesInBuffer, socketbridge::sDecodedMessage *out, u16 *out_nBytesConsumed) const
{
	//deve essere lungo almeno 7 byte e iniziare con #
	if (nBytesInBuffer < 7)
	{
		//scarta il messaggio perchè non puà² essere un msg valido
		DBGBREAK;
		*out_nBytesConsumed = nBytesInBuffer;
		return false;
	}

	if (bufferPt[0] != '#')
	{
		//msg non valido, avanzo fino a che non trovo un # oppure fine buffer
		for (u16 i = 1; i < nBytesInBuffer; i++)
		{
			if (bufferPt[i] == '#')
			{
				bool ret = priv_extractOneMessage (&bufferPt[i], (nBytesInBuffer - i), out, out_nBytesConsumed);
				*out_nBytesConsumed += i;
				return ret;
			}
		}

		//errore, non ho trovato un #, msg non valido
		DBGBREAK;
		*out_nBytesConsumed = nBytesInBuffer;
		return false;
	}


	out->opcode = (socketbridge::eOpcode)bufferPt[1];
	out->requestID = bufferPt[2];
	out->payloadLen = ((u16)bufferPt[3] * 256) + (u16)bufferPt[4];

	if (nBytesInBuffer < 7 + out->payloadLen)
	{
		//errore, non ci sono abbastanza byte nel buffer per l'intero msg
		DBGBREAK;
		*out_nBytesConsumed = nBytesInBuffer;
		return false;
	}

	out->payload = &bufferPt[5];

	//verifica della ck
	const u16 ck = ((u16)bufferPt[5 + out->payloadLen] * 256) + bufferPt[6 + out->payloadLen];
	if (ck != rhea::utils::simpleChecksum16_calc(bufferPt, 5 + out->payloadLen))
	{
		//il messaggio sembrava buono ma la ck non lo è, lo scarto.
		//Consumo il # e passo avanti
		bool ret = priv_extractOneMessage (&bufferPt[1], (nBytesInBuffer - 1), out, out_nBytesConsumed);
		*out_nBytesConsumed += 1;
		return ret;
	}


	*out_nBytesConsumed = (7 + out->payloadLen);
	return true;
}

//*****************************************************************
bool Core::priv_encodeMessageOfTypeEvent (socketbridge::eEventType eventType, const void *optionalData, u16 lenOfOptionalData, u8 *out_buffer, u16 *in_out_bufferLength)
{
	//calcola la dimensione minima necessaria in byte per il buffer
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
		memcpy(&out_buffer[n], optionalData, lenOfOptionalData);
		n += lenOfOptionalData;
	}

	*in_out_bufferLength = n;
	return true;
}

//*****************************************************************
void Core::sendTo(const HSokServerClient &h, const u8 *buffer, u32 nBytesToSend)
{
	server->client_writeBuffer(h, buffer, nBytesToSend);
}

//*****************************************************************
void Core::sendEvent (const HSokServerClient &h, socketbridge::eEventType eventType, const void *optionalData, u16 lenOfOptionalData)
{
	u16 n = SEND_BUFFER_SIZE;
	if (priv_encodeMessageOfTypeEvent(eventType, optionalData, lenOfOptionalData, sendBuffer, &n))
		sendTo(h, sendBuffer, n);
	else
	{
		RHEAFREE(localAllocator, sendBuffer);
		while (SEND_BUFFER_SIZE < n)
			SEND_BUFFER_SIZE += 1024;

		sendBuffer = (u8*)RHEAALLOC(localAllocator, SEND_BUFFER_SIZE);
		sendEvent(h, eventType, optionalData, lenOfOptionalData);
	}
}

//**************************************************************************
u16 Core::priv_getANewHandlerID ()
{
    _nextHandlerID++;

    //i primi RESERVED_HANDLE_RANGE id sono riservati ad uso interno
    if (_nextHandlerID < RESERVED_HANDLE_RANGE)
        _nextHandlerID = RESERVED_HANDLE_RANGE;
    return _nextHandlerID;
}

//**************************************************************************
void Core::priv_handleIdentification (const HSokServerClient &h, const socketbridge::sIdentifiedClientInfo *identifiedClient, socketbridge::sDecodedMessage &decoded)
{
	if (decoded.opcode == socketbridge::eOpcode_request_idCode && decoded.payloadLen == 4)
	{
		//il client mi sta chiedendo di inviargli un idCode univoco che lui userà  da ora in avanti per indentificarsi.
		//Mi sta anche già  comunicando la sua clientVer, che rimarrà  immutata di qui in avanti
		socketbridge::SokBridgeClientVer clientVer;
		clientVer.zero();
		clientVer.apiVersion = decoded.payload[0];
		clientVer.appType = decoded.payload[1];
		clientVer.unused2 = decoded.payload[2];
		clientVer.unused3 = decoded.payload[3];

		//Creo un nuovo record in idClientList e genero un idCode univoco
		socketbridge::SokBridgeIDCode	idCode;
		socketbridge::HSokBridgeClient identifiedCLientHanle = identifiedClientList.newEntry(rhea::getTimeNowMSec(), &idCode);
		identifiedClientList.updateClientVerInfo(identifiedCLientHanle, clientVer);

		logger->log("connection [0x%08X] is requesting new idCode. Sending [0x%08X]\n", h.asU32(), idCode.data.asU32);

		//rispodo al client inviandogli il suo idCode
		const u8 cpuBridgeVersion = 0x00;
		const u8 SOCKETBRIDGE_VERSION = 0x01;
		u8 toSend[8] = { cpuBridgeVersion, SOCKETBRIDGE_VERSION, idCode.data.buffer[0], idCode.data.buffer[1], idCode.data.buffer[2], idCode.data.buffer[3], 0 ,0 };
		sendEvent(h, socketbridge::eEventType_answer_to_idCodeRequest, toSend, 6);
		return;
	}
	else if (decoded.opcode == socketbridge::eOpcode_identify_W && decoded.payloadLen == 8)
	{
		assert(decoded.opcode == socketbridge::eOpcode_identify_W);

		socketbridge::SokBridgeClientVer clientVer;
		clientVer.zero();
		clientVer.apiVersion = decoded.payload[0];
		clientVer.appType = decoded.payload[1];
		clientVer.unused2 = decoded.payload[2];
		clientVer.unused3 = decoded.payload[3];

		socketbridge::SokBridgeIDCode	idCode;
		idCode.data.buffer[0] = decoded.payload[4];
		idCode.data.buffer[1] = decoded.payload[5];
		idCode.data.buffer[2] = decoded.payload[6];
		idCode.data.buffer[3] = decoded.payload[7];

		//ho ricevuto un idCode e una clientVer; questa combinazione deve esistere di già  nella mia lista di identifiedClient, altrimenti chiudo connession
		identifiedClient = identifiedClientList.isKnownIDCode(idCode);
		if (NULL == identifiedClient)
		{
			logger->log("refusing connection [0x%08X] because its idCode is not valid [idCode:0x%08X]\n", h.asU32(), idCode.data.asU32);
			server->client_sendClose(h);
			return;
		}

		if (!identifiedClientList.compareClientVerInfo(identifiedClient->handle, clientVer))
		{
			logger->log("refusing connection [0x%08X] because its clientVer is not equal to previous ver\n", h.asU32());
			server->client_sendClose(h);
			return;
		}

		//ok, se siamo arrivati qui vuol dire che il client mi ha inviato un idCode corretto e una clientVer corretta.
		//[identifiedClient] è quindi un valido pt. Bindo la socket al client e ritorno
		logger->log("client [0x%08X]: reconnected with connection [0x%08X] \n", idCode.data.asU32, h.asU32());
		if (!identifiedClientList.bindSocket(identifiedClient->handle, h, rhea::getTimeNowMSec()))
		{
			logger->log("warn, socket is already bind\n");
			identifiedClientList.unbindSocket(identifiedClient->handle, rhea::getTimeNowMSec());
			identifiedClientList.bindSocket(identifiedClient->handle, h, rhea::getTimeNowMSec());
		}
		return;
	}
	else
	{
		logger->log("ERR: killing connection [0x%08X] because it's not identified [rcvd opcode=%d]\n", h.asU32(), decoded.opcode);
		server->client_sendClose(h);
		return;
	}
}

/**************************************************************************
 * onClientHasDataAvail
 *
 * Un client già  collegato ha inviato dei dati lungo la socket
 */
void Core::priv_onClientHasDataAvail(u8 iEvent, u64 timeNowMSec)
{
	HSokServerClient h = server->getEventSrcAsClientHandle(iEvent);
	const socketbridge::sIdentifiedClientInfo	*identifiedClient = identifiedClientList.isKwnownSocket(h, timeNowMSec);

	i32 nBytesLetti = server->client_read(h, buffer);
	if (nBytesLetti == -1)
	{
		logger->log("connection [0x%08X] closed\n", h.asU32());

		//cerco il client che aveva questa socket e, se esiste, la unbindo.
		//Il client rimane registrato nella lista degli identificati, ma non ha una socket associata
		if (identifiedClient)
		{
			identifiedClientList.unbindSocket(identifiedClient->handle, rhea::getTimeNowMSec());
		}
		return;
	}

	if (nBytesLetti < 1)
	{
		logger->log("connection [0x%08X] not enought data [%d bytes]\n", h.asU32(), nBytesLetti);
		return;
	}

	//ho ricevuto un messaggio, proviamo a decodificarlo
	socketbridge::sDecodedMessage decoded;
	u16 nBytesConsumed;
	u8 *bufferPt = buffer._getPointer(0);
	while (priv_extractOneMessage(bufferPt, (u16)nBytesLetti, &decoded, &nBytesConsumed))
	{
		priv_onClientHasDataAvail2 (timeNowMSec, h, identifiedClient, decoded);

		assert(nBytesLetti >= nBytesConsumed);
		nBytesLetti -= nBytesConsumed;
		if (nBytesLetti == 0)
			return;
		bufferPt += nBytesConsumed;

		if (NULL == identifiedClient)
			identifiedClient = identifiedClientList.isKwnownSocket(h, timeNowMSec);
	}
}

void Core::priv_onClientHasDataAvail2(u64 timeNowMSec, HSokServerClient &h, const socketbridge::sIdentifiedClientInfo *identifiedClient, socketbridge::sDecodedMessage &decoded)
{
	//se la socket in questione non è stata ancora bindata ad un client, allora il cliente mi deve per
	//forza aver mandato un messaggio di tipo [eOpcode_request_idCode] oppure [eOpcode_identify_W], altrimenti killo la connessione.
	//Se invece la socket è già  bindata ad un identified-client, nessun problema, passo ad analizzare il messaggio perchè il client è stato già  identificato ad 
	//un giro precedente
	if (NULL == identifiedClient)
	{
		priv_handleIdentification(h, identifiedClient, decoded);
		return;
	}


	//ok, vediamo che cosa mi sta chiedendo il client
	//logger->log ("client [0x%08X]: rcv command [%c]\n", identifiedClient->idCode.data.asU32, decoded.opcode);
	switch (decoded.opcode)
	{
	default:
		logger->log("unkown command [%c]\n", decoded.opcode);
		return;

	case socketbridge::eOpcode_event_E:
		/*ho ricevuto una richiesta di tipo "scatena un evento"
		Istanzio un "handler" appropriato che possa gestire la richiesta. Se quest'handler esiste, allora ci sono 2 possibiità :
			1- la richiesta deve essere inoltrata alla GPU (per es mi stanno chiedendo un aggiornamento sullo stato di disp delle selezioni)
			2- la richiesta la gestisce direttamente this (per es mi stanno chiedendo una lista dei client connessi).
		Caso 1:
			aggiungo l'handle alla lista degli handler attivi e chiamo il suo metodo fwdRequestToGPU() che, a sua volta, invia la richiesta a GPU
			in modo che GPU la possa intercettare rispondere sulla stessa msgQ.
			La risposta di GPU (che prevede tra i vari parametri anche l'handlerID), viene gestita dallo stesso handler che ho istanziato qui che, sostanzialmente, spedisce indietro
			al client il risultato appena ottenuto da CPUBridge.
			Il giro quindi è:
				client chiede qualcosa a this
				this istanzia un handler che a sua volta gira la richiesta a GPU lungo la msgQ dedicata
				GPU riceve la richiesta e risponde a this lungo la stessa msgQ
				this recupera la risposta di GPU e la passa allo stesso handler che inizialmente si era preoccupato di girare la richiesta a GPU
				L'handler in questione fa le sue elaborazione e spedisce indietro il risultato al client

		Caso 2:
			chiamo direttamente il metodo handleRequestCore() dell'handler che risponde al cliente, e poi distruggo l'istanza di handler.
		*/
		if (decoded.payloadLen >= 1)
		{
			socketbridge::eEventType evType = (socketbridge::eEventType)decoded.payload[0];
			socketbridge::CmdHandler_eventReq *handler = socketbridge::CmdHandler_eventReqFactory::spawnFromSocketClientEventType(localAllocator, identifiedClient->handle, evType, priv_getANewHandlerID(), 10000);
			if (NULL != handler)
			{
				if (handler->needForwardToGPU())
				{
					cmdHandlerList.add(handler);
					handler->fwdRequestToGPU(decoded.payload, decoded.payloadLen);
				}
				else
				{
					handler->handleRequestCore(this, h);
					RHEADELETE(localAllocator, handler);
				}
			}
			else
			{
				logger->log ("socketListener::Core::onClientHasDataAvail2() => unsupported eOpcode_event_E [%d]\n", (u8)evType);
			}
		}
		break;

	case socketbridge::eOpcode_ajax_A:
		/*	ho ricevuto una richiesta di tipo "ajax"
			Funziona allo stesso modo di cui sopra
		*/
		{
			const u8 *params = NULL;
			socketbridge::CmdHandler_ajaxReq *handler = socketbridge::CmdHandler_ajaxReqFactory::spawn(localAllocator, identifiedClient->handle, decoded.requestID, decoded.payload, decoded.payloadLen, priv_getANewHandlerID(), 10000, &params);
			if (NULL != handler)
			{
				if (handler->needForwardToGPU())
				{
					cmdHandlerList.add(handler);
					handler->fwdRequestToGPU(params);
				}
				else
				{
					handler->handleRequestCore(this, h, params);
					RHEADELETE(localAllocator, handler);
				}
			}
#ifdef _DEBUG
			else
			{
				logger->log ("socketListener::Core::onClientHasDataAvail2() => unsupported eOpcode_ajax_A\n");
				for (u32 i = 0; i < decoded.payloadLen; i++)
					logger->log ("%c", decoded.payload[i]);
				logger->log ("\n");
			}
#endif
		}
		break;

	case socketbridge::eOpcode_fileTransfer:
		// non gestito in questa implementazione, vedi progetto socketBridge
		break;
	}
}

