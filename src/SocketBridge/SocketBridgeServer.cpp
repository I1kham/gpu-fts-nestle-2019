#include "SocketBridge.h"
#include "SocketBridgeVersion.h"
#include "SocketBridgeServer.h"
#include "../CPUBridge/CPUBridge.h"
#include "../rheaCommonLib/rheaString.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaLogTargetConsole.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "CmdHandler_ajaxReq.h"
#include "CmdHandler_eventReq.h"

using namespace socketbridge;


//***************************************************
Server::Server()
{
    localAllocator = NULL;
    server = NULL;
    logger = &nullLogger;
	eventSeqNumber = 0;
	cpuBridgeVersion = 0x00;
	rhea::event::setInvalid (hSubscriberEvent);
}

//***************************************************
void Server::useLogger (rhea::ISimpleLogger *loggerIN)
{
    if (NULL==loggerIN)
        logger = &nullLogger;
    else
        logger = loggerIN;

    if (NULL != server)
        server->useLogger(loggerIN);
}

//***************************************************
bool Server::open (u16 SERVER_PORT, const HThreadMsgW &hCPUServiceChannelW)
{
    logger->log ("SocketBridgeServer::open\n");
    logger->incIndent();

    localAllocator = RHEANEW(rhea::memory_getDefaultAllocator(), rhea::AllocatorSimpleWithMemTrack) ("socketBridgeSrv");

    server = RHEANEW(localAllocator, rhea::ProtocolSocketServer)(8, localAllocator);
    server->useLogger(logger);

	logger->log("opening socket...");
    eSocketError err = server->start (SERVER_PORT);
    if (err != eSocketError_none)
    {
        logger->log("FAIL\n");
        logger->decIndent();
        RHEADELETE(localAllocator, server);
        return false;
    }
    logger->log("OK\n");


	//iscrizione alla cpu
	if (!hCPUServiceChannelW.isInvalid())
	{
		logger->log("subscribing to CPUBridge...");
		if (!priv_subsribeToCPU(hCPUServiceChannelW))
		{
			logger->log("FAIL\n");
			logger->decIndent();
			RHEADELETE(localAllocator, server);
			return false;
		}
	}


    logger->decIndent();


    //elenco dei clienti identificati
    identifiedClientList.setup (localAllocator);

    //lista degli handler dei messaggi
    nextTimePurgeCmdHandlerList = 0;
    cmdHandlerList.setup (localAllocator);

    //linear buffer per la ricezione di msg dai client
    buffer.setup (localAllocator, 128);

    _nextHandlerID = 0;


	//gestore del file transfer
	fileTransfer.setup(localAllocator, logger);
    return true;
}

//***************************************************
void Server::close()
{
    if (NULL == localAllocator)
        return;

	fileTransfer.unsetup();

    if (NULL != server)
    {
		cpubridge::unsubscribe (subscriber);

        buffer.unsetup();
        cmdHandlerList.unsetup();
        identifiedClientList.unsetup ();

        server->close();
        RHEADELETE(localAllocator, server);
    }

    RHEADELETE(rhea::memory_getDefaultAllocator(), localAllocator);
    logger->log ("SocketBridgeServer::closed\n");
}

//***************************************************
bool Server::priv_subsribeToCPU(const HThreadMsgW &hCPUServiceChannelW)
{
	assert(rhea::event::isInvalid(hSubscriberEvent));

	//creo una msgQ temporanea per ricevere da CPUBridge la risposta alla mia richiesta di iscrizione
	HThreadMsgR hMsgQR;
	HThreadMsgW hMsgQW;
	rhea::thread::createMsgQ (&hMsgQR, &hMsgQW);

	//invio la richiesta
	cpubridge::subscribe (hCPUServiceChannelW, hMsgQW);

	//attendo risposta
	u64 timeToExitMSec = rhea::getTimeNowMSec() + 2000;
	do
	{
		rhea::thread::sleepMSec(50);

		rhea::thread::sMsg msg;
		if (rhea::thread::popMsg(hMsgQR, &msg))
		{
			//ok, ci siamo
			if (msg.what == CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER)
			{
				cpubridge::translate_SUBSCRIPTION_ANSWER (msg, &subscriber, &cpuBridgeVersion);
				rhea::thread::deleteMsg(msg);
				rhea::thread::getMsgQEvent(subscriber.hFromCpuToOtherR, &hSubscriberEvent);
				server->addOSEventToWaitList (hSubscriberEvent, u32MAX);
				break;
			}

			rhea::thread::deleteMsg(msg);
		}
	} while (rhea::getTimeNowMSec() < timeToExitMSec);
	
	//delete della msgQ
	rhea::thread::deleteMsgQ (hMsgQR, hMsgQW);
	

	return rhea::event::isValid(hSubscriberEvent);
}

//*****************************************************************
bool Server::priv_decodeMessage(rhea::LinearBuffer &buffer, u16 *in_out_offset, u16 bufferSizeInByte, socketbridge::sDecodedMessage *out)
{
	if (*in_out_offset >= bufferSizeInByte)
		return false;
	
	const u16 bytesLeft = bufferSizeInByte - (*in_out_offset);
	u8 *bufferPt = buffer._getPointer(*in_out_offset);

	//deve essere lungo almeno 7 byte e iniziare con #
	if (bytesLeft < 7 || bufferPt[0] != '#')
		return false;


	out->opcode = (eOpcode)bufferPt[1];
	out->requestID = bufferPt[2];
	out->payloadLen = ((u16)bufferPt[3] * 256) + (u16)bufferPt[4];

	if (bytesLeft < 7 + out->payloadLen)
		return false;

	out->payload = &bufferPt[5];

	//verifica della ck
	const u16 ck = ((u16)bufferPt[5 + out->payloadLen] * 256) + bufferPt[6 + out->payloadLen];
	if (ck != rhea::utils::simpleChecksum16_calc (bufferPt, 5 + out->payloadLen))
		return false;

	const u16 totalSizeOfThisMsg = out->payloadLen + 7;	
	(*in_out_offset) += totalSizeOfThisMsg;

	assert (bufferSizeInByte == totalSizeOfThisMsg);
	return true;
}

//*****************************************************************
bool Server::priv_encodeMessageOfTypeEvent (eEventType eventType, const void *optionalData, u16 lenOfOptionalData, u8 *out_buffer, u16 *in_out_bufferLength)
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
bool Server::priv_encodeMessageOfAjax(u8 requestID, const char *ajaxData, u16 lenOfAjaxData, u8 *out_buffer, u16 *in_out_bufferLength)
{
	//calcola la dimensione minima necessaria in byte per il buffer
	const u16 minSizeInBytes = 6 + lenOfAjaxData;

	if (*in_out_bufferLength < minSizeInBytes)
	{
		DBGBREAK;
		*in_out_bufferLength = minSizeInBytes;
		return false;
	}

	u16 n = 0;
	out_buffer[n++] = '#';
	out_buffer[n++] = 'A';
	out_buffer[n++] = 'J';
	out_buffer[n++] = requestID;
	out_buffer[n++] = 'j';
	out_buffer[n++] = 'a';

	if (lenOfAjaxData)
	{
		memcpy(&out_buffer[n], ajaxData, lenOfAjaxData);
		n += lenOfAjaxData;
	}

	*in_out_bufferLength = n;
	return true;
}


//*****************************************************************
void Server::sendTo(const HSokServerClient &h, const u8 *buffer, u32 nBytesToSend)
{
	server->client_writeBuffer(h, buffer, nBytesToSend);
}

//*****************************************************************
void Server::sendAjaxAnwer (const HSokServerClient &h, u8 requestID, const char *ajaxData, u16 lenOfAjaxData)
{
	const u16 BUFFER_SIZE = 1024;
	u8 buffer[BUFFER_SIZE];

	u16 n = BUFFER_SIZE;
	if (priv_encodeMessageOfAjax (requestID, ajaxData, lenOfAjaxData, buffer, &n))
		sendTo(h, buffer, n);
}

//*****************************************************************
void Server::sendEvent (const HSokServerClient &h, eEventType eventType, const void *optionalData, u16 lenOfOptionalData)
{
	const u16 BUFFER_SIZE = 1024;
	u8 buffer[BUFFER_SIZE];

	u16 n = BUFFER_SIZE;
	if (priv_encodeMessageOfTypeEvent(eventType, optionalData, lenOfOptionalData, buffer, &n))
		sendTo(h, buffer, n);
}

//***************************************************
void Server::formatSinglePrice(u16 price, char *out, u16 sizeofOut) const
{
	const char DECIMAL_SEP = '.';
	const u8 NUM_DECIMAL = 2;
	rhea::string::format::currency (price, NUM_DECIMAL, DECIMAL_SEP, out, sizeofOut);
}

//***************************************************
void Server::formatPriceList (const u16 *priceList, u16 nPricesInList, char *out, u16 sizeofOut) const
{
	out[0] = 0x00;
	if (sizeofOut < 2)
		return;
	sizeofOut--;
	
	for (u16 i = 0; i < nPricesInList; i++)
	{
		char s[32];
		formatSinglePrice (priceList[i], s, sizeof(s));

		u8 len = 1 + (u8)strlen(s);
		if (sizeofOut < len)
			return;

		if (i > 0)
			strcat(out, "§");
		strcat(out, s);
		sizeofOut -= len;
	}
}


//***************************************************
void Server::run()
{
    assert (server != NULL);

    bQuit = false;
    while (bQuit == false)
    {
        //la wait() si sveglia se è in arrivo una nuova connessione, se un client già connesso invia dati oppure se la coda di msg con CPUBridge ha qualche msg pendente
        u8 nEvents = server->wait (5000);

        //elimino eventuali handler rimasti pendenti
        u64 timeNowMSec = rhea::getTimeNowMSec();
        if (timeNowMSec > nextTimePurgeCmdHandlerList)
        {
            cmdHandlerList.purge (localAllocator, timeNowMSec);
			identifiedClientList.purge(timeNowMSec);
			nextTimePurgeCmdHandlerList = timeNowMSec + 10000;
        }

		//aggiornamento degli eventuali file trnasfer in corso
		fileTransfer.update(timeNowMSec);

        //analizzo gli eventi ricevuti
        for (u8 i=0; i<nEvents; i++)
        {
            switch (server->getEventType(i))
            {
            default:
                logger->log ("SocketBridgeServer::run(), unknown event type: %d\n", server->getEventType(i));
                break;

			case rhea::ProtocolSocketServer::evt_ignore:
				break;

            case rhea::ProtocolSocketServer::evt_new_client_connected:
                {
                    //un client vuole connettersi. Accetto la socket in ingresso, ma mi riservo di chiudere la
                    //connessione se il prossimo msg che manda è != dal messaggio di identificazione.
                    //HSokServerClient h = server->getEventSrcAsClientHandle(i);
                    //printf ("server> new connection accepted (handle:0x%02X)\n", h.asU32());
                }
                break;

            case rhea::ProtocolSocketServer::evt_client_has_data_avail:
				//uno dei clienti già connessi ha mandato qualcosa
                priv_onClientHasDataAvail (i, timeNowMSec);
                break;

            case rhea::ProtocolSocketServer::evt_osevent_fired:
                {
					//un OSEvent associato alla waitList è stato attivato.
					//Al momento la cosa significa solo che CPUBridge ha mandato un msg lungo la msgQ
					//In futuro, potrebbero esserci anche altri eventi da monitorare.
					const u32 fromWhom = server->getEventSrcUserParam(i);

					switch (fromWhom)
					{
					case u32MAX:
						//E' arrivato un msg da CPUBridge
						rhea::thread::sMsg msg;
						while (rhea::thread::popMsg(subscriber.hFromCpuToOtherR, &msg))
						{
							priv_onCPUBridgeNotification(msg);
							rhea::thread::deleteMsg(msg);
						}
						break;

					default:
						logger->log("SocketBridgeServer => unkwnown event from waitList\n");
						break;
					}
                }
                break;
            }
        }
    }

    cmdHandlerList.purge (localAllocator, u64MAX);
}

/**************************************************************************
 * priv_getANewHandlerID ()
 *
 * Ritorna un ID univoco da assegnare agli handler dei messaggi
 */
u16 Server::priv_getANewHandlerID ()
{
    _nextHandlerID++;

    //i primi RESERVED_HANDLE_RANGE id sono riservati ad uso interno
    if (_nextHandlerID < RESERVED_HANDLE_RANGE)
        _nextHandlerID = RESERVED_HANDLE_RANGE;
    return _nextHandlerID;
}


//**************************************************************************
void Server::priv_handleIdentification (const HSokServerClient &h, const sIdentifiedClientInfo *identifiedClient, socketbridge::sDecodedMessage &decoded)
{
	if (decoded.opcode == socketbridge::eOpcode_request_idCode && decoded.payloadLen == 4)
	{
		//il client mi sta chiedendo di inviargli un idCode univoco che lui userà da ora in avanti per indentificarsi.
		//Mi sta anche già comunicando la sua clientVer, che rimarrà immutata di qui in avanti
		SokBridgeClientVer clientVer;
		clientVer.zero();
		clientVer.apiVersion = decoded.payload[0];
		clientVer.appType = decoded.payload[1];
		clientVer.unused2 = decoded.payload[2];
		clientVer.unused3 = decoded.payload[3];

		//Creo un nuovo record in idClientList e genero un idCode univoco
		SokBridgeIDCode	idCode;
		HSokBridgeClient identifiedCLientHanle = identifiedClientList.newEntry(rhea::getTimeNowMSec(), &idCode);
		identifiedClientList.updateClientVerInfo(identifiedCLientHanle, clientVer);

		logger->log("connection [0x%08X] is requesting new idCode. Sending [0x%08X]\n", h.asU32(), idCode.data.asU32);

		//rispodo al client inviandogli il suo idCode
		u32 version = (((u32)cpuBridgeVersion) << 8) | SOCKETBRIDGE_VERSION;
		u8 toSend[8] = { cpuBridgeVersion, SOCKETBRIDGE_VERSION, idCode.data.buffer[0], idCode.data.buffer[1], idCode.data.buffer[2], idCode.data.buffer[3], 0 ,0 };
		sendEvent(h, eEventType_answer_to_idCodeRequest, toSend, 6);
		return;
	}
	else if (decoded.opcode == socketbridge::eOpcode_identify_W && decoded.payloadLen == 8)
	{
		assert(decoded.opcode == socketbridge::eOpcode_identify_W);

		SokBridgeClientVer clientVer;
		clientVer.zero();
		clientVer.apiVersion = decoded.payload[0];
		clientVer.appType = decoded.payload[1];
		clientVer.unused2 = decoded.payload[2];
		clientVer.unused3 = decoded.payload[3];

		SokBridgeIDCode	idCode;
		idCode.data.buffer[0] = decoded.payload[4];
		idCode.data.buffer[1] = decoded.payload[5];
		idCode.data.buffer[2] = decoded.payload[6];
		idCode.data.buffer[3] = decoded.payload[7];

		//ho ricevuto un idCode e una clientVer; questa combinazione deve esistere di già nella mia lista di identifiedClient, altrimenti chiudo connession
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
			logger->log("error, socket is already bind\n");
		}
		else
		{
			//gli mando l'attuale messaggio di CPU (uso la stessa procedura che userei se fosse stato davvero il client a chiedermelo)
			CmdHandler_eventReq *handler = CmdHandler_eventReqFactory::spawnFromSocketClientEventType(localAllocator, identifiedClient->handle, eEventType_cpuMessage, priv_getANewHandlerID(), 10000);
			if (NULL != handler)
			{
				cmdHandlerList.add(handler);
				handler->passDownRequestToCPUBridge(subscriber, NULL, 0);
			}

		}
		return;
	}
	else
	{
		logger->log("killing connection [0x%08X] because it's not identified\n", h.asU32());
		server->client_sendClose(h);
		return;
	}
}

/**************************************************************************
 * onClientHasDataAvail
 *
 * Un client già collegato ha inviato dei dati lungo la socket
 */
void Server::priv_onClientHasDataAvail (u8 iEvent, u64 timeNowMSec)
{
	HSokServerClient h = server->getEventSrcAsClientHandle(iEvent);
	const sIdentifiedClientInfo	*identifiedClient = identifiedClientList.isKwnownSocket(h);

    i32 nBytesLetti = server->client_read (h, buffer);
    if (nBytesLetti == -1)
    {
        logger->log ("connection [0x%08X] closed\n", h.asU32());
		
		//cerco il client che aveva questa socket e, se esiste, la unbindo.
		//Il client rimane registrato nella lista degli identificati, ma non ha una socket associata
		if (identifiedClient)
			identifiedClientList.unbindSocket (identifiedClient->handle, rhea::getTimeNowMSec());
		return;
    }

	if (nBytesLetti < 1)
	{
		logger->log("connection [0x%08X] not enought data [%d bytes]\n", h.asU32(), nBytesLetti);
		return;
	}

    //ho ricevuto un messaggio, proviamo a decodificarlo
	socketbridge::sDecodedMessage decoded;
	u16 bufferOffset = 0;
	while (priv_decodeMessage(buffer, &bufferOffset, (u16)nBytesLetti, &decoded))
	{
		//se la socket in questione non è stata ancora bindata ad un client, allora il cliente mi deve per
		//forza aver mandato un messaggio di tipo [eOpcode_request_idCode] oppure [eOpcode_identify_W], altrimenti killo la connessione.
		//Se invece la socket è già bindata ad un identified-client, nessun problema, passo ad analizzare il messaggio perchè il client è stato già identificato ad 
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
			Istanzio un "handler" appropriato che possa gestire la richiesta. Se quest'handler esiste, allora ci sono 2 possibiità:
				1- la richiesta deve essere passata a CPUBrdige (per es mi stanno chiedendo un aggiornamento sullo stati di disp delle selezioni)
				2- la richiesta la gestisce direttamente this (per es mi stanno chiedendo una lita dei client connessi).
			Caso 1:
				aggiungo l'handle alla lista degli handler attivi e chiamo il suo metodo passDownRequestToCPUBridge() che, a sua volta, appende una richiesta alla msgQ
				verso CPUBridge in modo che CPUBridge possa intercettare la richiesta e rispondere sulla stessa msgQ.
				La risposta di CPUBridge (che prevede tra i vari parametri anche l'handlerID), viene gestita dallo stesso handler che ho istanziato qui che, sostanzialmente, spedisce indietro
				al client il risultato appena ottenuto da CPUBridge.
				Il giro quindi è:
					client chiede qualcosa a SocketBridge
					SocketBridge istanzia un handler che a sua volta gira la richiesta a CPUBridge lungo la msgQ dedicata
					CPUBridge riceve la richiesta e risponde a SocketBridge lungo la stessa msgQ
					SocketBridge recupera la risposta di CPUBridge e la passa allo stesso handler che inizialmente si era preoccupato di girare la richiesta a CPUBridge
					L'handler in questione fa le sue elaborazione e spedisce indietro il risultato al client

			Caso 2:
				chiamo direttamente il metodo handleRequestFromSocketBridge() dell'handler che risponde al cliente, e poi distruggo l'istanza di handler.
			*/
			if (decoded.payloadLen >= 1)
			{
				socketbridge::eEventType evType = (socketbridge::eEventType)decoded.payload[0];
				CmdHandler_eventReq *handler = CmdHandler_eventReqFactory::spawnFromSocketClientEventType(localAllocator, identifiedClient->handle, evType, priv_getANewHandlerID(), 10000);
				if (NULL != handler)
				{
					if (handler->needToPassDownToCPUBridge())
					{
						cmdHandlerList.add(handler);
						handler->passDownRequestToCPUBridge(subscriber, decoded.payload, decoded.payloadLen);
					}
					else
					{
						handler->handleRequestFromSocketBridge(this, h);
						RHEADELETE(localAllocator, handler);
					}
				}
			}
			break;

		case socketbridge::eOpcode_ajax_A:
			/*	ho ricevuto una richiesta di tipo "ajax"
				Funziona allo stesso modo di cui sopra, solo che non è previsto un comando che debba essere gestito da this.
				Tutte le richieste di questo tipo sono da passare a CPUBrdige
			*/
			{
				const char *params = NULL;
				CmdHandler_ajaxReq *handler = CmdHandler_ajaxReqFactory::spawn(localAllocator, identifiedClient->handle, decoded.requestID, decoded.payload, decoded.payloadLen, priv_getANewHandlerID(), 10000, &params);
				if (NULL != handler)
				{
					cmdHandlerList.add(handler);
					handler->passDownRequestToCPUBridge(subscriber, params);
				}
			}
			break;

		case socketbridge::eOpcode_fileTransfer:
			/*	messaggio di gestione dei file transfer */
			fileTransfer.handleMsg(this, h, decoded, timeNowMSec);
			break;
		}
	}
}


/**************************************************************************
 * priv_onCPUBridgeNotification
 *
 * E' arrivato un messaggio da parte di CUPBrdige sulla msgQ dedicata (ottenuta durante la subscribe di this a CPUBridge).
 */
void Server::priv_onCPUBridgeNotification (rhea::thread::sMsg &msg)
{
	const u16 handlerID = (msg.paramU32 & 0x0000FFFF);
    const u8 *data = (const u8*)msg.buffer;

	if (handlerID == 0)
	{
		//in queso caso, CPUBridge ha mandato una notifica di sua spontanea volontà, non è una risposta ad una mia specifica richiesta.
		//Questo vuol dire che devo diffondere la notifica a tutti i miei client connessi
		rhea::Allocator *allocator = rhea::memory_getScrapAllocator();

		const u16 notifyID = (u16)msg.what;
		for (u32 i = 0; i < identifiedClientList.getCount(); i++)
		{
			HSokBridgeClient identifiedClientHandle;
			if (identifiedClientList.getHandleByIndex(i, &identifiedClientHandle))
			{
				HSokServerClient h;
				if (identifiedClientList.isBoundToSocket(identifiedClientHandle, &h))
				{
					//BRUTTO allocare ogni volta... forse al posto di handleAnswer() meglio splittare in prepare / send / free in modo da chiamare 1 volta sola prepare, n volte send e 1 free
					CmdHandler_eventReq *handler = CmdHandler_eventReqFactory::spawnFromCPUBridgeEventID(allocator, identifiedClientHandle, notifyID, 0, 10000);
					if (NULL != handler)
					{
						handler->onCPUBridgeNotification(this, h, msg);
						RHEADELETE(allocator, handler);
					}
				}
			}
		}
	}
	else
	{
		//in questo caso, CPUBridge ha risposto ad richiesta iniziata da me tramite un handler che io stesso ho istanziato (vedi priv_onClientHasDataAvail).
		//Recupero l'handler appropriato e gestisco la risposta.
		CmdHandler *handler = cmdHandlerList.findByHandlerID (handlerID);
		if (NULL != handler)
		{
			const HSokBridgeClient identifiedClientHandle = handler->getIdentifiedClientHandle();
			HSokServerClient h;
			if (identifiedClientList.isBoundToSocket(identifiedClientHandle, &h))
			{
				handler->onCPUBridgeNotification(this, h, msg);
				cmdHandlerList.removeAndDeleteByID(localAllocator, handlerID);
			}
			else
			{
				//segno che questo client ha un messaggio pendente, bisogna spedirglielo alla sua riconnessione
				//identifiedClientList.markAsHavPendingRequest(identifiedClientHandle, true);
			}
		}
	}
}
