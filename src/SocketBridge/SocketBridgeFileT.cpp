#include "SocketBridgeFileT.h"
#include "SocketBridgeServer.h"
#include "../rheaCommonLib/rheaUtils.h"

using namespace socketbridge;


//***************************************************
FileTransfer::FileTransfer()
{
	localAllocator = NULL;
	logger = NULL;
	bufferW = NULL;
}

//***************************************************
void FileTransfer::setup (rhea::Allocator *allocator, rhea::ISimpleLogger *loggerIN)
{
	localAllocator = allocator;
	logger = loggerIN;
	bufferW = (u8*)RHEAALLOC(localAllocator, SIZE_OF_BUFFERW);
	activeTransferList.setup(localAllocator, 32);
}

//***************************************************
void FileTransfer::unsetup()
{
	if (NULL == localAllocator)
		return;

	u32 n = activeTransferList.getNElem();
	for (u32 i = 0; i < n; i++)
		priv_freeResources(i);
	activeTransferList.unsetup();

	RHEAFREE(localAllocator, bufferW);
	localAllocator = NULL;
}


//***************************************************
void FileTransfer::priv_send (Server *server, const HSokServerClient &h, const u8 *what, u16 sizeofwhat)
{
	assert(SIZE_OF_BUFFERW >= 8 + sizeofwhat);
	u32 ct = 0;
	bufferW[ct++] = '#'; 
	bufferW[ct++] = 'f';
	bufferW[ct++] = 'T';
	bufferW[ct++] = 'r';
	bufferW[ct++] = ((sizeofwhat & 0xFF00) >> 8);
	bufferW[ct++] =  (sizeofwhat & 0x00FF);
	
	if (sizeofwhat)
	{
		memcpy(&bufferW[ct], what, sizeofwhat);
		ct += sizeofwhat;
	}

	const u16 ck = rhea::utils::simpleChecksum16_calc(bufferW, ct);
	bufferW[ct++] = ((ck & 0xFF00) >> 8);
	bufferW[ct++] = (ck & 0x00FF);

	server->sendTo(h, bufferW, ct);
}

//***************************************************
u32 FileTransfer::priv_generateUID() const
{
	u32 ret;
	bool bQuit = false;
	while (bQuit == false)
	{
		const u32 a = 1 + rhea::randomU32(254);
		const u32 b = 1 + rhea::randomU32(254);
		const u32 c = 1 + rhea::randomU32(254);
		const u32 d = 1 + rhea::randomU32(254);
		ret = (a << 24) | (b << 16) | (c << 8) | d;

		bQuit = true;
		for (u32 i = 0; i < activeTransferList.getNElem(); i++)
		{
			if (activeTransferList(i).smuFileTransfUID == ret)
			{
				bQuit = false;
				break;
			}
		}
	}
	return ret;
}

//***************************************************
u32 FileTransfer::priv_findActiveTransferBySMUUID(u32 smuFileTransfUID) const
{
	const u32 n = activeTransferList.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		if (activeTransferList(i).smuFileTransfUID == smuFileTransfUID)
			return i;
	}
	return u32MAX;
}

//***************************************************
void FileTransfer::priv_freeResources(u32 i)
{
	if (NULL != activeTransferList[i].f)
	{
		fclose(activeTransferList[i].f);
		activeTransferList[i].f = NULL;
	}
}

//***************************************************
void FileTransfer::update(u64 timeNowMSec)
{
	u32 n = activeTransferList.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		switch (activeTransferList(i).status)
		{
		case eTransferStatus_finished_OK:
			logger->log("Transfert [0x%08X] finished OK\n", activeTransferList(i).smuFileTransfUID);
			priv_freeResources(i);
			activeTransferList.removeAndSwapWithLast(i);
			--i;
			--n;
			break;

		case eTransferStatus_finished_KO:
			logger->log("Transfert [0x%08X] finished KO\n", activeTransferList(i).smuFileTransfUID);
			priv_freeResources(i);
			activeTransferList.removeAndSwapWithLast(i);
			--i;
			--n;
			break;

		case eTransferStatus_pending:
			if (timeNowMSec > activeTransferList(i).timeoutMSec)
			{
				activeTransferList[i].status = eTransferStatus_finished_KO;
				logger->log("Transfert [0x%08X] time out\n", activeTransferList(i).smuFileTransfUID);
			}
			break;
		}
	}
}

//***************************************************
void FileTransfer::handleMsg (Server *server, const HSokServerClient &h, socketbridge::sDecodedMessage &decoded, u64 timeNowMSec)
{
	if (decoded.payloadLen == 0)
	{
		DBGBREAK;
		return;
	}

	rhea::NetStaticBufferViewR nbr;
	nbr.setup(decoded.payload, decoded.payloadLen, rhea::eBigEndian);

	u8 opcode;
	nbr.readU8(opcode);
	switch ((eFileTransferOpcode)opcode)
	{
	default:
		logger->log("ERR: FileTransfer::handleMsg() unkwnown opcode [0x%02X]\n", opcode);
		return;

	case eFileTransferOpcode_upload_request_fromApp:
		//qualcuno vuole iniziare un upload verso di me
		priv_on_Opcode_upload_request_fromApp (server, h, nbr, timeNowMSec);
		break;

	case eFileTransferOpcode_upload_request_fromApp_packet:
		priv_on_Opcode_upload_request_fromApp_packet(server, h, nbr, timeNowMSec);
		break;
	}
}

//***************************************************
void FileTransfer::priv_on_Opcode_upload_request_fromApp (Server *server, const HSokServerClient &h, rhea::NetStaticBufferViewR &nbr, u64 timeNowMSec)
{
	//decodifica e validazione dei dati ricevuti
	fileT::sData0x01 data;
	if (!data.decode(nbr))
	{
		DBGBREAK;
		return;
	}


	//preparo la risposta
	fileT::sData0x02 answ;
	answ.reason_refused = 0;
	answ.packetSizeInBytes = 0;
	answ.smuTransfUID = 0;
	answ.appTransfUID = data.appTransfUID;


	//ok... qualcuno vuole iniziare un upload verso di me
	//verifichiamo che il suo "usage" sia lecito
	FILE *fDest = NULL;
	while (1)
	{
		if (strcasecmp(data.usage, "test") == 0)
		{
			char s[256];
			sprintf_s(s, sizeof(s), "%s/test.upload", rhea::getPhysicalPathToWritableFolder());
			fDest = fopen(s, "wb");
			if (NULL == fDest)
				answ.reason_refused = (u8)eFileTransferFailReason_smuErrorOpeningFile;
			break;
		}

		break;
	}


	if (NULL != fDest)
	{
		const u16 PACKET_SIZE = 512;
		//genero un UID per il trasferimento
		//aggiungo il recordo alla lista dei trasferimenti attivi
		sActiveUploadRequest info;
		info.status = eTransferStatus_pending;
		info.f = fDest;
		info.smuFileTransfUID = priv_generateUID();
		info.appFileTransfUID = data.appTransfUID;
		info.timeoutMSec = timeNowMSec + 5000;
		info.nextTimeOutputANotificationMSec = 0;
		info.packetSize = PACKET_SIZE;
		info.lastGoodPacket = u32MAX;
		info.totalFileSizeInBytes = data.fileSizeInBytes;
		info.numOfPacketToBeRcvInTotal = data.fileSizeInBytes / info.packetSize;
		if (info.numOfPacketToBeRcvInTotal * info.packetSize < data.fileSizeInBytes)
			info.numOfPacketToBeRcvInTotal++;

		activeTransferList.append(info);

		//preparo la risposta
		answ.reason_refused = 0;
		answ.packetSizeInBytes = info.packetSize;
		answ.smuTransfUID = info.smuFileTransfUID;
	}
	else
	{
		logger->log("ERR: FileTransfer::priv_on_Opcode_upload_request_fromApp() invalid usage [%s] or error opening file\n", data.usage);
		return;
	}


	
	//spedisco
	u8	toSendBuffer[128];
	u16 sizeOfToSendBuffer = answ.encode(toSendBuffer, sizeof(toSendBuffer));
	priv_send (server, h, toSendBuffer, sizeOfToSendBuffer);


	logger->log("FileTransfer => accepted, smuUID[0x%08X] appUID[0x%08X]\n", answ.smuTransfUID, answ.appTransfUID);
}


//***************************************************
void FileTransfer::priv_on_Opcode_upload_request_fromApp_packet (Server *server, const HSokServerClient &h, rhea::NetStaticBufferViewR &nbr, u64 timeNowMSec)
{
	u32 smuFileTransfUID, packetNumReceived;
	nbr.readU32(smuFileTransfUID);
	nbr.readU32(packetNumReceived);

	//ho ricevuto un pacchetto dati, vediamo se lo riconosco
	u32 index = priv_findActiveTransferBySMUUID(smuFileTransfUID);
	if (u32MAX == index)
	{
		//sconosciuto...
		DBGBREAK;
		return;
	}

	sActiveUploadRequest *req = &activeTransferList.getElem(index);

	if (req->status != eTransferStatus_pending)
	{
		DBGBREAK;
		return;
	}

	//aggiorno il timeout per questo tranfert
	req->timeoutMSec = timeNowMSec + 14000;

	//vediamo se il pacchetto ricevuto è coerente
	u32 expectedPacketNum = req->lastGoodPacket + 1;

#ifdef _DEBUG
	/*simulo un errore di trasferimento
	if (rhea::random01() < 0.05f)
	{
		expectedPacketNum++;
	}*/
#endif


	if (packetNumReceived == expectedPacketNum)
	{
		//bene, era quello buono
		

		//aggiorno le info sul trasferimento
		req->lastGoodPacket = packetNumReceived;

		//per non spammare messagi, ne butto fuori uno ogni 5 secondi
		if (timeNowMSec > req->nextTimeOutputANotificationMSec)
		{
			logger->log("FileTransfer => [0x%08X] packet [%d]/[%d]\n", req->smuFileTransfUID, 1 + req->lastGoodPacket, req->numOfPacketToBeRcvInTotal);
			req->nextTimeOutputANotificationMSec = timeNowMSec + 1000;
		}

		//era l'ultimo pacchetto?
		if (req->lastGoodPacket + 1 >= req->numOfPacketToBeRcvInTotal)
		{
			u32 lastPacketSize = req->totalFileSizeInBytes - req->lastGoodPacket * req->packetSize;
			nbr.readBlob(bufferW, lastPacketSize);
			fwrite(bufferW, lastPacketSize, 1, req->f);

			req->status = eTransferStatus_finished_OK;
			fclose(req->f);
			req->f = NULL;
		}
		else
		{
			nbr.readBlob(bufferW, req->packetSize);
			fwrite(bufferW, req->packetSize, 1, req->f);
		}


		
	}
	else
	{
		//mi è arrivato un pacchetto che non era quello che mi aspettato (li voglio in ordine seqeuenziale).
		//Confermo la ricezione per evitare il timeout, ma come "packetNumAccepted" spedisco il mio "last good packet"
		//in modo che dall'altra parte ripartano a spedire pacchetti da dove dico io
	}


	//rispondo confermando la ricezione
	fileT::sData0x04 answ;
	answ.appTransfUID = req->appFileTransfUID;
	answ.packetNumAccepted = req->lastGoodPacket;

	//spedisco
	u8	toSendBuffer[128];
	u16 sizeOfToSendBuffer = answ.encode(toSendBuffer, sizeof(toSendBuffer));
	priv_send(server, h, toSendBuffer, sizeOfToSendBuffer);
}
