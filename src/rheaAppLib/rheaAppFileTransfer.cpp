#include "rheaApp.h"
#include "rheaAppFileTransfer.h"
#include "../rheaCommonLib/rheaUtils.h"


using namespace rhea;

using namespace rhea::app;

#define STATUS_IN_PROGRESS	0x00
#define STATUS_FINISHED_OK	0x01
#define STATUS_FINISHED_KO	0x02

#define WHATAMIDOING_NONE					0x00
#define WHATAMIDOING_UPLOAD_REQUEST_SENT	0x10		//ho chiesto a socketbridge se vuole accettare questo file, sono in attesa di risposta
#define WHATAMIDOING_UPLOADING				0x11
#define WHATAMIDOING_DOWNLOAD_REQUEST_SENT	0x20		//ho chiesto a socketbridge di mandarmi un file
#define WHATAMIDOING_DOWNLOADING			0x21

//**********************************************************************
void FileTransfer::setup(rhea::Allocator *allocatorIN, rhea::ISimpleLogger *loggerIN)
{
	assert(localAllocator == NULL);
	localAllocator = allocatorIN;
	logger = loggerIN;

	handleArray.setup(localAllocator, MAX_SIMULTANEOUS_TRANSFER, sizeof(sRecord));
	activeHandleList.setup(localAllocator, MAX_SIMULTANEOUS_TRANSFER);
	eventList.setup(localAllocator);
	bufferReadFromFile = (u8*)RHEAALLOC(localAllocator, SIZE_OF_READ_BUFFER_FROM_FILE);
}

//**********************************************************************
void FileTransfer::unsetup()
{
	if (NULL == localAllocator)
		return;

	RHEAFREE(localAllocator, bufferReadFromFile);

	//free delle cose in sospeso
	u32 n = activeHandleList.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		Handle h = activeHandleList(i);

		sRecord *s;
		if (!handleArray.fromHandleToPointer(h, &s))
			continue;

		priv_freeResources(s);
		handleArray.dealloc(h);
	}

	handleArray.unsetup();
	activeHandleList.unsetup();
	eventList.unsetup();

	localAllocator = NULL;
}

//**********************************************************************
bool FileTransfer::priv_isUploading(const sRecord *s) const
{
	return (s->isAnUpload != 0);
}

//**********************************************************************
void FileTransfer::priv_freeResources(sRecord *s) const
{
	if (NULL != s->f)
	{
		fclose(s->f);
		s->f = NULL;
	}

	if (priv_isUploading(s))
	{
		//era un upload
		if (NULL != s->other.whenUploading.sendBuffer)
		{
			RHEAFREE(localAllocator, s->other.whenUploading.sendBuffer);
			s->other.whenUploading.sendBuffer = NULL;
			s->other.whenUploading.sizeOfSendBuffer = 0;
		}
	}
	else
	{
		//era un download
	}
}

//**********************************************************************
void FileTransfer::priv_failed (sRecord *s, socketbridge::eFileTransferFailReason failReason, bool bAppendFailEvent)
{
	s->status = STATUS_FINISHED_KO;
	s->whatAmIDoing = (u8)failReason;
	s->timeoutMSec = 0;

	if (bAppendFailEvent)
	{
		//aggiungo un evento di uscita per segnalare che il transfert è terminato malamente
		sTansferInfo ev;
		priv_fillTransferInfo(s, &ev);
		eventList.push(ev);
	}
}

//**********************************************************************
bool FileTransfer::getTransferInfo (const Handle &h, sTansferInfo *out)
{
	sRecord *s;
	if (!handleArray.fromHandleToPointer(h, &s))
		return false;

	priv_fillTransferInfo(s, out);
	return true;
}

//**********************************************************************
void FileTransfer::priv_fillTransferInfo (const sRecord *s, sTansferInfo *out) const
{
	out->timeElapsedMSec = rhea::getTimeNowMSec() - s->timeStartedMSec;
	out->handle = s->handle;
	out->totalTransferSizeInBytes = s->fileSizeInBytes;
	
	if (priv_isUploading(s))
	{
		out->currentTransferSizeInBytes = s->packetSize * s->other.whenUploading.packetNum;
	}
	else
	{
		out->currentTransferSizeInBytes = s->packetSize * s->other.whenDownloading.lastGoodPacket;
	}
	
	switch (s->status)
	{
	case STATUS_IN_PROGRESS:
		out->status = app::eFileTransferStatus_inProgress; 
		out->failReason = socketbridge::eFileTransferFailReason_none;
		break;
	
	case STATUS_FINISHED_OK: 
		out->status = app::eFileTransferStatus_finished_OK; 
		out->failReason = socketbridge::eFileTransferFailReason_none;
		out->currentTransferSizeInBytes = s->fileSizeInBytes;
		break;
	
	default:
	case STATUS_FINISHED_KO:
		out->status = app::eFileTransferStatus_finished_KO; 
		out->failReason = (socketbridge::eFileTransferFailReason)s->whatAmIDoing;
		break;
	}
}

//**********************************************************************
FileTransfer::sRecord* FileTransfer::priv_fromHandleAsU32(u32 hAsU32) const
{
	FileTransfer::Handle h;
	h.initFromU32(hAsU32);
	
	sRecord *s;
	if (handleArray.fromHandleToPointer(h, &s))
		return s;

	//handle invalido.... non dovrebbe mai succedere
	return NULL;
}

//**********************************************************************
void FileTransfer::onMessage (u64 timeNowMSec, const sDecodedFileTransfMsg &msg)
{
	rhea::NetStaticBufferViewR nbr;
	nbr.setup(msg.payload, msg.payloadLen, rhea::eBigEndian);

	u8 opcode; 
	nbr.readU8(opcode);
	switch ((socketbridge::eFileTransferOpcode)opcode)
	{
	default:
		DBGBREAK;
		//logger->log("ERR: FileTransfer::onMessage() => invalid opcode [%d]\n", opcode);
		break;

	case socketbridge::eFileTransferOpcode_upload_request_fromApp_answ:				priv_on0x02(timeNowMSec, nbr); break;
	case socketbridge::eFileTransferOpcode_upload_request_fromApp_packet_answ:		priv_on0x04(timeNowMSec, nbr); break;

	case socketbridge::eFileTransferOpcode_download_request_fromApp_answ:			priv_on0x52(timeNowMSec, nbr); break;
	case socketbridge::eFileTransferOpcode_download_request_fromApp_packet:			priv_on0x53(timeNowMSec, nbr); break;
	}
}

//**********************************************************************
bool FileTransfer::update(u64 timeNowMSec)
{
	u32 n = activeHandleList.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		Handle h = activeHandleList(i);

		sRecord *s;
		if (!handleArray.fromHandleToPointer(h, &s))
		{
			//handle invalido.... non dovrebbe mai succedere
			DBGBREAK;
			activeHandleList.removeAndSwapWithLast(i);
			--i;
			--n;
			continue;
		}

		//se è in stato di "finito", lo rimuovo dalla lista degli activeHandler
		if (s->status == STATUS_FINISHED_OK || s->status == STATUS_FINISHED_KO)
		{
			//genero l'evento
			sTansferInfo ev;
			priv_fillTransferInfo(s, &ev);
			eventList.push(ev);

			//libero risorse
			priv_freeResources(s);
			activeHandleList.removeAndSwapWithLast(i);
			--i;
			--n;

			handleArray.dealloc(h);
			continue;
		}

		//vediamo se è in timeout
		if (timeNowMSec >= s->timeoutMSec)
		{
			//libero le risorse, genero un evento e rimuovo l'handler dalla activeList
			priv_failed (s, socketbridge::eFileTransferFailReason_timeout, true);
			activeHandleList.removeAndSwapWithLast(i);
			--i;
			--n;

			handleArray.dealloc(h);
			continue;
		}

	}

	return (eventList.isEmpty() == false);
}




//**********************************************************************
bool FileTransfer::startFileUpload (rhea::IProtocolChannell *ch, rhea::IProtocol *proto, u64 timeNowMSec, const char *fullFileNameAndPath, const char *usage, Handle *out_handle)
{
	if (fullFileNameAndPath == NULL || usage == NULL)
		return false;
	if (fullFileNameAndPath[0] == 0x00 || usage[0] == 0x00)
		return false;

	const u32 usageLen = (u32)strlen(usage);
	if (usageLen >= socketbridge::fileT::sData0x01::MAX_USAGE_LEN)
		return false;

	//vediamo se il file esiste
	FILE *f = fopen(fullFileNameAndPath, "rb");
	if (NULL == f)
		return false;

	//pongo un limite alla dimensione massima ragionevole di un transferimento
	const u64 UN_MEGA = 1024 * 1024;
	u64 fileSizeInBytes = rhea::fs::filesize(f);
	if (fileSizeInBytes == 0 || fileSizeInBytes > 256 * UN_MEGA || fileSizeInBytes >= u32MAX)
	{
		fclose(f);
		return false;
	}

	//pare tutto a posto, alloco un nuovo handle e invio la richiesta a socketBridge per vedere se è disponibile ad accettare il mio file
	sRecord *s = handleArray.allocIfYouCan();
	if (NULL == s)
	{
		DBGBREAK;
		fclose(f);
		return false;
	}

	//aggiungo l'handle alla lista degli handle attivi
	activeHandleList.append(s->handle);

	//setup della struttura dati necessaria al trasferimento
	const u16 PROPOSED_PACKET_SIZE = 1024;
	const u16 PROPOSED_NUM_PACKET_IN_A_CHUNK = 32;
	s->f = f;
	s->ch = ch;
	s->proto = proto;
	s->smuFileTransfUID = 0;
	s->timeStartedMSec = timeNowMSec;
	s->timeoutMSec = timeNowMSec + 4000; //se entro 4 secondi non mi risponde, abortisco l'operazione
	s->nextTimePushAnUpdateMSec = 0;
	s->packetSize = PROPOSED_PACKET_SIZE;
	s->numPacketInAChunk = PROPOSED_NUM_PACKET_IN_A_CHUNK;
	s->fileSizeInBytes = (u32)fileSizeInBytes;
	s->status = STATUS_IN_PROGRESS;
	s->whatAmIDoing = WHATAMIDOING_UPLOAD_REQUEST_SENT;
	s->isAnUpload = 1;

	s->other.whenUploading.sizeOfSendBuffer = 0;
	s->other.whenUploading.sendBuffer = NULL;
	s->other.whenUploading.packetNum = 0;
	s->other.whenUploading.numOfPacketToBeSentInTotal = 0;
	


	*out_handle = s->handle;


	//preparo la richiesta a SMU
	socketbridge::fileT::sData0x01 data;
	data.usageLen = usageLen;
	data.packetSizeInBytes = s->packetSize;
	data.fileSizeInBytes = s->fileSizeInBytes;
	data.appTransfUID = s->handle.asU32();
	data.numPacketInAChunk = s->numPacketInAChunk;
	memcpy(data.usage, usage, usageLen + 1);

	//spedisco
	u8 serializedDataBuffer[128];
	u16 sizeOfSerializedDataBuffer = data.encode(serializedDataBuffer, sizeof(serializedDataBuffer));

	u8 sendBuffer[128];
	app::RawFileTrans::sendToSocketBridge (ch, proto, sendBuffer, sizeof(sendBuffer), serializedDataBuffer, sizeOfSerializedDataBuffer);
	return true;
}

//**********************************************************************
void FileTransfer::priv_sendChunkOfPackets(sRecord *s, u16 nPacket)
{
	if (s->status != STATUS_IN_PROGRESS)
		return;
	if (s->whatAmIDoing != WHATAMIDOING_UPLOADING)
		return;

	if (s->packetSize > (SIZE_OF_READ_BUFFER_FROM_FILE - 32))
	{
		DBGBREAK;
		priv_failed(s, socketbridge::eFileTransferFailReason_localReadBufferTooShort, true);
		return;
	}

	rhea::NetStaticBufferViewW nbw;
	nbw.setup(bufferReadFromFile, SIZE_OF_READ_BUFFER_FROM_FILE, rhea::eBigEndian);

	u32 iPacket = s->other.whenUploading.packetNum;
	u32 fileOffset = iPacket * s->packetSize;
	u32 nBytesLeft = s->fileSizeInBytes - fileOffset;

	u8 chunkSeq = 1;
	fseek(s->f, fileOffset, SEEK_SET);
	while (nPacket-- && nBytesLeft)
	{
		u16 nByteToRead = s->packetSize;
		if (nBytesLeft < nByteToRead)
		{
			nByteToRead = (u16)nBytesLeft;
			nBytesLeft = 0;
		}
		else
			nBytesLeft -= nByteToRead;

		nbw.seek(0, rhea::eSeekStart);
		nbw.writeU8((u8)socketbridge::eFileTransferOpcode_upload_request_fromApp_packet);
		nbw.writeU32(s->smuFileTransfUID);
		nbw.writeU32(iPacket);
		nbw.writeU8(chunkSeq);
		fread(&bufferReadFromFile[nbw.length()], nByteToRead, 1, s->f);

		//logger->log("FT => sending packet [%d]\n", s->packetNum);
		app::RawFileTrans::sendToSocketBridge(s->ch, s->proto, s->other.whenUploading.sendBuffer, s->other.whenUploading.sizeOfSendBuffer, bufferReadFromFile, nbw.length() + nByteToRead);

		iPacket++;
		chunkSeq++;
	}
}

//**********************************************************************
void FileTransfer::priv_on0x02 (u64 timeNowMSec, rhea::NetStaticBufferViewR &nbr)
{
	socketbridge::fileT::sData0x02 data;
	if (!data.decode(nbr))
	{
		DBGBREAK; 
		//logger->log("ERR: FileTransfer::priv_on0x02() => decoding failed\n");
		return;
	}

	sRecord *s = priv_fromHandleAsU32(data.appTransfUID);
	if (NULL == s)
	{
		DBGBREAK;
		return;
	}

	if (data.reason_refused != 0)
	{
		//SMU ha rifiutato la mia richiesta
		priv_failed (s, socketbridge::eFileTransferFailReason_smuRefused, true);
		return;
	}

	//tutto bene, inziamo a mandare pacchetti
	s->status = STATUS_IN_PROGRESS;
	s->whatAmIDoing = WHATAMIDOING_UPLOADING;
	s->smuFileTransfUID = data.smuTransfUID;
	s->timeoutMSec = timeNowMSec + 5000;
	s->packetSize = data.packetSizeInBytes;
	s->numPacketInAChunk = data.numPacketInAChunk;
	s->other.whenUploading.sizeOfSendBuffer = s->packetSize + 32;
	s->other.whenUploading.sendBuffer = (u8*)RHEAALLOC(localAllocator, s->other.whenUploading.sizeOfSendBuffer);

	//calcolo il num totale di pacchetti che si dovranno inviare
	s->other.whenUploading.numOfPacketToBeSentInTotal = s->fileSizeInBytes / s->packetSize;
	if (s->other.whenUploading.numOfPacketToBeSentInTotal * s->packetSize < s->fileSizeInBytes)
		s->other.whenUploading.numOfPacketToBeSentInTotal++;

	//genero un evento per segnalare l'inizio dell'upload
	sTansferInfo ev;
	priv_fillTransferInfo(s, &ev);
	eventList.push(ev);

	//invio iul primo pacchetto
	priv_sendChunkOfPackets(s, s->numPacketInAChunk);
}

//**********************************************************************
void FileTransfer::priv_on0x04 (u64 timeNowMSec, rhea::NetStaticBufferViewR &nbr)
{
	socketbridge::fileT::sData0x04 data;
	if (!data.decode(nbr))
	{
		DBGBREAK;
		//logger->log("ERR: FileTransfer::priv_on0x02() => decoding failed\n");
		return;
	}

	sRecord *s = priv_fromHandleAsU32(data.appTransfUID);
	if (NULL == s)
	{
		DBGBREAK;
		return;
	}

	//SMU ha accettato un pacchetto, aggiorno il mio stato
	s->other.whenUploading.packetNum = data.packetNumAccepted + 1;
	s->timeoutMSec = timeNowMSec + 15000;

	//vediamo se era l'ultimo
	if (s->other.whenUploading.packetNum >= s->other.whenUploading.numOfPacketToBeSentInTotal)
	{
		//ho finito!!
		s->status = STATUS_FINISHED_OK;
		s->whatAmIDoing = (u8)socketbridge::eFileTransferFailReason_none;
		s->timeoutMSec = 0;

		//aggiungo un evento di uscita per segnalare che il transfert è terminato
		sTansferInfo ev;
		priv_fillTransferInfo(s, &ev);
		eventList.push(ev);
		return;
	}

	//genero un evento per segnalare il corretto invio di un pacco
	//Dato che non voglio spammare eventi, genero un evento al primo pacchetto e poi ogni tot, in base al tempo passatto dall'ultima notifica
	if (timeNowMSec > s->nextTimePushAnUpdateMSec)
	{
		sTansferInfo ev;
		priv_fillTransferInfo(s, &ev);
		eventList.push(ev);
		s->nextTimePushAnUpdateMSec = timeNowMSec + 5000;
	}

	//invio il prossimo pacco
	priv_sendChunkOfPackets(s, s->numPacketInAChunk);
}




//**********************************************************************
bool FileTransfer::startFileDownload(rhea::IProtocolChannell *ch, rhea::IProtocol *proto, u64 timeNowMSec, const char *what, const char *dstFullFileNameAndPath, Handle *out_handle)
{
	if (dstFullFileNameAndPath == NULL || what == NULL)
		return false;
	if (dstFullFileNameAndPath[0] == 0x00 || what[0] == 0x00)
		return false;

	const u32 whatLen = (u32)strlen(what);
	if (whatLen >= socketbridge::fileT::sData0x51::MAX_WHAT_LEN)
		return false;

	//provo ad aprire il file in scrittura
	FILE *f = fopen(dstFullFileNameAndPath, "wb");
	if (NULL == f)
		return false;

	//pare tutto a posto, alloco un nuovo handle e invio la richiesta a socketBridge
	sRecord *s = handleArray.allocIfYouCan();
	if (NULL == s)
	{
		DBGBREAK;
		fclose(f);
		return false;
	}

	//aggiungo l'handle alla lista degli handle attivi
	activeHandleList.append(s->handle);

	//setup della struttura dati necessaria al trasferimento
//	const u16 PROPOSED_PACKET_SIZE = 1024;
	s->f = f;
	s->ch = ch;
	s->proto = proto;
	s->smuFileTransfUID = 0;
	s->timeoutMSec = timeNowMSec + 4000; //se entro 4 secondi non mi risponde, abortisco l'operazione
	s->timeStartedMSec = timeNowMSec;
	s->status = STATUS_IN_PROGRESS;
	s->whatAmIDoing = WHATAMIDOING_DOWNLOAD_REQUEST_SENT;
	s->isAnUpload = 0;
	
	s->other.whenDownloading.lastGoodPacket = u32MAX;
	s->other.whenDownloading.numOfPacketToBeRcvInTotal = 0;


	*out_handle = s->handle;


	//*preparo la richiesta a SMU
	socketbridge::fileT::sData0x51 data;
	data.whatLen = whatLen;
	data.appTransfUID = s->handle.asU32();
	memcpy(data.what, what, whatLen + 1);

	//spedisco
	u8 serializedDataBuffer[128];
	u16 sizeOfSerializedDataBuffer = data.encode(serializedDataBuffer, sizeof(serializedDataBuffer));

	u8 sendBuffer[128];
	app::RawFileTrans::sendToSocketBridge(ch, proto, sendBuffer, sizeof(sendBuffer), serializedDataBuffer, sizeOfSerializedDataBuffer);
	return true;
}

//**********************************************************************
void FileTransfer::priv_on0x52(u64 timeNowMSec, rhea::NetStaticBufferViewR &nbr)
{
	socketbridge::fileT::sData0x52 data;
	if (!data.decode(nbr))
	{
		DBGBREAK;
		return;
	}

	sRecord *s = priv_fromHandleAsU32(data.appTransfUID);
	if (NULL == s)
	{
		DBGBREAK;
		return;
	}

	if (data.reason_refused != 0)
	{
		//SMU ha rifiutato la mia richiesta
		priv_failed(s, socketbridge::eFileTransferFailReason_smuRefused, true);
		return;
	}

	//tutto bene, memorizzo le info sul transfer
	s->status = STATUS_IN_PROGRESS;
	s->whatAmIDoing = WHATAMIDOING_DOWNLOADING;
	s->timeoutMSec = timeNowMSec + 5000;
	s->smuFileTransfUID = data.smuTransfUID;
	s->packetSize = data.packetSizeInBytes;
	s->numPacketInAChunk = data.numPacketInAChunk;
	s->fileSizeInBytes = data.fileSize;
	
	s->other.whenDownloading.nextTimeSendNACKMsec = 0;
	s->other.whenDownloading.lastGoodPacket = u32MAX;
	s->other.whenDownloading.numOfPacketToBeRcvInTotal = s->fileSizeInBytes / s->packetSize;
	if (s->other.whenDownloading.numOfPacketToBeRcvInTotal * s->packetSize < s->fileSizeInBytes)
		s->other.whenDownloading.numOfPacketToBeRcvInTotal++;
		

	//invio una ack per far partire il download
	socketbridge::fileT::sData0x54 answ;
	answ.smuFileTransfUID = s->smuFileTransfUID;
	answ.packetNumAccepted = u32MAX;


	//spedisco
	u8 serializedDataBuffer[128];
	u16 sizeOfSerializedDataBuffer = answ.encode(serializedDataBuffer, sizeof(serializedDataBuffer));

	u8 sendBuffer[128];
	app::RawFileTrans::sendToSocketBridge(s->ch, s->proto, sendBuffer, sizeof(sendBuffer), serializedDataBuffer, sizeOfSerializedDataBuffer);
}

//***********************************************************************
void FileTransfer::priv_on0x53(u64 timeNowMSec, rhea::NetStaticBufferViewR &nbr)
{
	//ho ricevuto un pacchetto
    u32 appFileTransfUID = 0, packetNumReceived = 0;
    u8	chunkSeq = 0;
	nbr.readU32(appFileTransfUID);
	nbr.readU32(packetNumReceived);
	nbr.readU8(chunkSeq);

	sRecord *s = priv_fromHandleAsU32(appFileTransfUID);
	if (NULL == s)
	{
		DBGBREAK;
		return;
	}

	if (s->status != STATUS_IN_PROGRESS)
		return;

	//aggiorno il timeout per questo tranfert
	s->timeoutMSec = timeNowMSec + 15000;

	//vediamo se il pacchetto ricevuto è coerente
	u32 expectedPacketNum = s->other.whenDownloading.lastGoodPacket + 1;


	if (packetNumReceived == expectedPacketNum)
	{
		//bene, era quello buono


		//aggiorno le info sul trasferimento
		s->other.whenDownloading.lastGoodPacket = packetNumReceived;

		//per non spammare messagi a video, ne butto fuori uno ogni 5 secondi
		if (timeNowMSec > s->nextTimePushAnUpdateMSec)
		{
			sTansferInfo ev;
			priv_fillTransferInfo(s, &ev);
			eventList.push(ev);
			s->nextTimePushAnUpdateMSec = timeNowMSec + 5000;
		}

		//era l'ultimo pacchetto?
		if (s->other.whenDownloading.lastGoodPacket + 1 >= s->other.whenDownloading.numOfPacketToBeRcvInTotal)
		{
			u32 lastPacketSize = s->fileSizeInBytes - s->other.whenDownloading.lastGoodPacket * s->packetSize;
			nbr.readBlob(bufferReadFromFile, lastPacketSize);
			fwrite(bufferReadFromFile, lastPacketSize, 1, s->f);

			s->status = STATUS_FINISHED_OK;
			fclose(s->f);
			s->f = NULL;
		}
		else
		{
			nbr.readBlob(bufferReadFromFile, s->packetSize);
			fwrite(bufferReadFromFile, s->packetSize, 1, s->f);
		}

		//rispondo confermando la ricezione
		s->other.whenDownloading.nextTimeSendNACKMsec = 0;
		if (chunkSeq == s->numPacketInAChunk || s->status == STATUS_FINISHED_OK)
		{
			socketbridge::fileT::sData0x54 answ;
			answ.smuFileTransfUID = s->smuFileTransfUID;
			answ.packetNumAccepted = s->other.whenDownloading.lastGoodPacket;

			//spedisco
			u8 serializedDataBuffer[128];
			u16 sizeOfSerializedDataBuffer = answ.encode(serializedDataBuffer, sizeof(serializedDataBuffer));

			u8 sendBuffer[128];
			app::RawFileTrans::sendToSocketBridge(s->ch, s->proto, sendBuffer, sizeof(sendBuffer), serializedDataBuffer, sizeOfSerializedDataBuffer);
			logger->log("FileTransfer => [0x%08X] send ACK for packet[%d]\n", s->handle.asU32(), s->other.whenDownloading.lastGoodPacket + 1);
		}
	}
	else
	{
		//mi è arrivato un pacchetto che non era quello che mi aspettato (li voglio in ordine seqeuenziale).
		//Confermo la ricezione per evitare il timeout, ma come "packetNumAccepted" spedisco il mio "last good packet"
		//in modo che dall'altra parte ripartano a spedire pacchetti da dove dico io
		if (timeNowMSec >= s->other.whenDownloading.nextTimeSendNACKMsec)
		{
			s->other.whenDownloading.nextTimeSendNACKMsec = timeNowMSec + 2000; //sta cosa serve per evitare di mandare 1000 NACK di seguito visto che presumibilmente c'è
																					//un intero chunk di pacchetti in arrivo che sto per scartare
			chunkSeq = s->numPacketInAChunk;

			socketbridge::fileT::sData0x54 answ;
			answ.smuFileTransfUID = s->smuFileTransfUID;
			answ.packetNumAccepted = s->other.whenDownloading.lastGoodPacket;

			//spedisco
			//spedisco
			u8 serializedDataBuffer[128];
			u16 sizeOfSerializedDataBuffer = answ.encode(serializedDataBuffer, sizeof(serializedDataBuffer));

			u8 sendBuffer[128];
			app::RawFileTrans::sendToSocketBridge(s->ch, s->proto, sendBuffer, sizeof(sendBuffer), serializedDataBuffer, sizeOfSerializedDataBuffer);

			logger->log("FileTransfer => [0x%08X] send NACK for packet[%d]\n", s->handle.asU32(), s->other.whenDownloading.lastGoodPacket + 1);
		}
	}
}
