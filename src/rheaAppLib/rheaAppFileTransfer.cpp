#include "rheaApp.h"
#include "rheaAppFileTransfer.h"
#include "../rheaCommonLib/rheaUtils.h"


using namespace rhea;

using namespace rhea::app;

#define STATUS_IN_PROGRESS	0x00
#define STATUS_FINISHED_OK	0x01
#define STATUS_FINISHED_KO	0x02

#define WHATAMIDOING_NONE					0x00
#define WHATAMIDOING_UPLOAD_REQUEST_SENT	0x01		//ho chiesto a socketbridge se vuole accettare questo file, sono in attesa di risposta
#define WHATAMIDOING_UPLOADING				0x02

//**********************************************************************
void FileTransfer::setup(rhea::Allocator *allocatorIN)
{
	assert(localAllocator == NULL);
	localAllocator = allocatorIN;

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
void FileTransfer::priv_freeResources(sRecord *s) const
{
	if (NULL != s->f)
	{
		fclose(s->f);
		s->f = NULL;
	}

	if (NULL != s->sendBuffer)
	{
		RHEAFREE (localAllocator, s->sendBuffer);
		s->sendBuffer = NULL;
		s->sizeOfSendBuffer = 0;
	}
}

//**********************************************************************
void FileTransfer::priv_failed (sRecord *s, socketbridge::eFileTransferFailReason failReason, bool bAppendFailEvent)
{
	priv_freeResources(s);
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
void FileTransfer::priv_fillTransferInfo(const sRecord *s, sTansferInfo *out) const
{
	out->totalTransferSizeInBytes = s->fileSizeInBytes;
	out->currentTransferSizeInBytes = s->packetSize * s->packetNum;
	out->handle = s->handle;
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
	u64 fileSizeInBytes = rhea::utils::filesize(f);
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
	s->f = f;
	s->ch = ch;
	s->proto = proto;
	s->sizeOfSendBuffer = 0;
	s->sendBuffer = NULL;
	s->smuFileTransfUID = 0;
	s->fileSizeInBytes = (u32)fileSizeInBytes;
	s->packetNum = 0;
	s->numOfPacketToBeSentInTotal = 0;
	s->timeoutMSec = timeNowMSec + 2000; //se entro 2 secondi non mi risponde, abortisco l'operazione
	s->nextTimePushAnUpdateMSec = 0;
	s->packetSize = PROPOSED_PACKET_SIZE;
	s->status = STATUS_IN_PROGRESS;
	s->whatAmIDoing = WHATAMIDOING_UPLOAD_REQUEST_SENT;
	


	*out_handle = s->handle;


	//preparo la richiesta a SMU
	socketbridge::fileT::sData0x01 data;
	data.usageLen = usageLen;
	data.packetSizeInBytes = s->packetSize;
	data.fileSizeInBytes = s->fileSizeInBytes;
	data.appTransfUID = s->handle.asU32();
	memcpy(data.usage, usage, usageLen + 1);

	//spedisco
	u8 optionalData[128];
	u16 sizeOfOptionalData = data.encode(optionalData, sizeof(optionalData));

	u8 sendBuffer[128];
	app::RawFileTrans::sendToSocketBridge (ch, proto, sendBuffer, sizeof(sendBuffer), optionalData, sizeOfOptionalData);
	return true;
}


//**********************************************************************
void FileTransfer::priv_on0x02(u64 timeNowMSec, rhea::NetStaticBufferViewR &nbr)
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
	s->packetSize = data.packetSizeInBytes;
	s->sizeOfSendBuffer = s->packetSize + 32;
	s->sendBuffer = (u8*)RHEAALLOC(localAllocator, s->sizeOfSendBuffer);
	s->timeoutMSec = timeNowMSec + 5000;

	//calcolo il num totale di pacchetti che si dovranno inviare
	s->numOfPacketToBeSentInTotal = s->fileSizeInBytes / s->packetSize;
	if (s->numOfPacketToBeSentInTotal * s->packetSize < s->fileSizeInBytes)
		s->numOfPacketToBeSentInTotal++;

	//genero un evento per segnalare l'inizio dell'upload
	sTansferInfo ev;
	priv_fillTransferInfo(s, &ev);
	eventList.push(ev);

	//invio iul primo pacchetto
	priv_advanceAndSendPacket(s);
}

//**********************************************************************
void FileTransfer::priv_advanceAndSendPacket(sRecord *s)
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


	//preparo il pacchetto
	const u32 nBytesSentSoFar = s->packetNum * s->packetSize;
	const u32 nBytesLeft = s->fileSizeInBytes - nBytesSentSoFar;
	
	rhea::NetStaticBufferViewW nbw;
	nbw.setup (bufferReadFromFile, SIZE_OF_READ_BUFFER_FROM_FILE, rhea::eBigEndian);

	nbw.writeU8((u8)socketbridge::eFileTransferOpcode_upload_request_fromApp_packet);
	nbw.writeU32(s->smuFileTransfUID);
	nbw.writeU32(s->packetNum);

	//leggo da file
	u16 nByteToRead = s->packetSize;
	if (nBytesLeft < nByteToRead)
		nByteToRead = (u16)nBytesLeft;
	fseek(s->f, nBytesSentSoFar, SEEK_SET);
	fread(&bufferReadFromFile[nbw.length()], nByteToRead, 1, s->f);

	app::RawFileTrans::sendToSocketBridge (s->ch, s->proto, s->sendBuffer, s->sizeOfSendBuffer, bufferReadFromFile, nbw.length() + nByteToRead);
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
	s->packetNum = data.packetNumAccepted + 1;
	s->timeoutMSec = timeNowMSec + 4000;

	//vediamo se era l'ultimo
	if (s->packetNum >= s->numOfPacketToBeSentInTotal)
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
	priv_advanceAndSendPacket(s);
}