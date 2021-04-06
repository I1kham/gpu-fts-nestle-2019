#include "ESAPIModuleRasPI.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../CPUBridge/CPUBridge.h"
#include "ESAPI.h"
#include "ESAPIProtocol.h"

using namespace esapi;

//********************************************************
ModuleRasPI::ModuleRasPI()
{
    fileUpload.f = NULL;
    fileUpload.lastTimeUpdatedMSec = 0;
    stato = ModuleRasPI::eStato::boot;
}

//********************************************************
bool ModuleRasPI::setup (sShared *shared)
{
    //buffer
	rs232BufferOUT = (u8*)RHEAALLOC(shared->localAllocator, SIZE_OF_RS232BUFFEROUT);
    sokBuffer = (u8*)RHEAALLOC(shared->localAllocator, SIZE_OF_SOKBUFFER);

    //socket list
    sockettList.setup (shared->localAllocator, 128);

    //stato corrente
    stato = ModuleRasPI::eStato::boot;
    shared->logger->log ("esapi::ModuleRasPI:: now in BOOT mode...\n");
	return true;
}

//********************************************************
void ModuleRasPI::priv_unsetup(sShared *shared)
{
    RHEAFREE(shared->localAllocator, rs232BufferOUT);
    RHEAFREE(shared->localAllocator, sokBuffer);
    sockettList.unsetup ();
}


//********************************************************
void ModuleRasPI::virt_handleMsgFromServiceQ (sShared *shared UNUSED_PARAM, const rhea::thread::sMsg &msg UNUSED_PARAM)
{
    //non c'è nulla che questo modulo debba gestire in caso di messaggi ricevuti da altri thread sulla msgQ
    DBGBREAK;
}

//********************************************************
void ModuleRasPI::virt_handleMsgFromSubscriber (sShared *shared, sSubscription &sub, const rhea::thread::sMsg &msg, u16 handlerID)
{
    switch (stato)
    {
    case ModuleRasPI::eStato::boot:      boot_handleMsgFromSubscriber (shared, sub, msg, handlerID); break;
    case ModuleRasPI::eStato::running:   running_handleMsgFromSubscriber (shared, sub, msg, handlerID); break;
    default:
        DBGBREAK;
        break;
    }
}

//********************************************************
void ModuleRasPI::virt_handleMsgFromCPUBridge (sShared *shared UNUSED_PARAM, cpubridge::sSubscriber &sub UNUSED_PARAM, const rhea::thread::sMsg &msg UNUSED_PARAM, u16 handlerID UNUSED_PARAM)
{
    //in stato boot, non ci sono notiche CPUBridge che devo gestire, ma non è un errore se ne ricevo

}

//********************************************************
void ModuleRasPI::virt_handleMsg_R_fromRs232 (sShared *shared, sBuffer *b)
{
    switch (stato)
    {
    case ModuleRasPI::eStato::boot:      boot_handleMsg_R_fromRs232 (shared, b); break;
    case ModuleRasPI::eStato::running:   running_handleMsg_R_fromRs232 (shared, b); break;
    default:
        DBGBREAK;
        break;
    }
}

//********************************************************
void ModuleRasPI::virt_handleMsgFromSocket (sShared *shared, OSSocket &sok, u32 userParam)
{
    switch (stato)
    {
    case ModuleRasPI::eStato::boot:      
        //non dovrebbe mai accadere
        DBGBREAK;
        break;

    case ModuleRasPI::eStato::running:  
        running_handleMsgFromSocket (shared, sok, userParam);
        break;

    default:
        DBGBREAK;
        break;
    }
}

//*******************************************************
ModuleRasPI::sConnectedSocket* ModuleRasPI::priv_2280_findConnectedSocketByUID (u32 uid)
{
	for (u32 i = 0; i < sockettList.getNElem(); i++)
	{
		if (sockettList(i).uid == uid)
			return &sockettList[i];
	}
	return NULL;
}

//***************************************************************
void ModuleRasPI::priv_2280_onClientDisconnected (sShared *shared, OSSocket &sok, u32 uid)
{
	for (u32 i = 0; i < sockettList.getNElem(); i++)
	{
		if (sockettList(i).uid == uid)
		{
			assert (rhea::socket::compare(sockettList(i).sok, sok));

			shared->waitableGrp.removeSocket (sockettList[i].sok);
			rhea::socket::close(sockettList[i].sok);
			sockettList.removeAndSwapWithLast(i);

			//comunico la disconnessione via seriale
            const u32 n = esapi::buildMsg_R0x02_closeSocket (uid, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
			shared->protocol->rs232_write (rs232BufferOUT, n);
            shared->logger->log ("esapi::ModuleRasPI::socket[%d] disconnected\n", uid);
			return;
		}
	}

	//non ho trovato il client nella lista...
	DBGBREAK;
}


/********************************************************************
 *
 *  stato = eStato_boot
 *
 *  In questa fase il modulo rasPI a me collegato è slave, attende mie eventuali richieste.
 *  Questo stato finisce (e si passa in stato_running) quando un thread iscritto alla mia q (es: il thread di GPU), mi manda una notifica
 *  ESAPI_ASK_RASPI_START. A quel punto, io dico al rasPI di entrare in stato running e io stesso esco da boot e vado in running
 */

//*********************************************************
void ModuleRasPI::boot_handleMsgFromSubscriber(sShared *shared, sSubscription &sub, const rhea::thread::sMsg &msg, u16 handlerID)
{
    switch (msg.what)
    {
    default:
        shared->logger->log("ModuleRasPI::boot_handleMsgFromSubscriber() => invalid msg.what [0x%02X]\n", msg.what);
        break;

    case ESAPI_ASK_RASPI_GET_IPandSSID:
        //chiedo al rasPI IP e SSID e poi notifico il mio subscriber
        {
            u32 ct = 0;
            rs232BufferOUT[ct++] = '#';
            rs232BufferOUT[ct++] = 'R';
            rs232BufferOUT[ct++] = 0x02;
            rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc(rs232BufferOUT, 3);
            ct++;
            shared->protocol->rs232_write (rs232BufferOUT, ct);

            //la risposta è a lunghezza variabile
            //# R [0x02] [ip1] [ip2] [ip3] [ip4] [lenSSID] [ssidString...] [ck]
            if (priv_boot_waitAnswer(shared, 'R', 0x02, 9, 8, rs232BufferOUT, 1000))
            {
                const char *ssid = (const char*)&rs232BufferOUT[8];
                rs232BufferOUT[8 + rs232BufferOUT[7]] = 0x00;
                notify_RASPI_WIFI_IPandSSID (sub.q, handlerID, shared->logger, rs232BufferOUT[3], rs232BufferOUT[4], rs232BufferOUT[5], rs232BufferOUT[6], ssid);
            }
        }
        break;

    case ESAPI_ASK_RASPI_START:
        //dico al rasPI di "startare"
        {
            u32 ct = 0;
            rs232BufferOUT[ct++] = '#';
            rs232BufferOUT[ct++] = 'R';
            rs232BufferOUT[ct++] = 0x01;
            rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc(rs232BufferOUT, ct);
            ct++;
            shared->protocol->rs232_write (rs232BufferOUT, ct);
            if (priv_boot_waitAnswer(shared, 'R', 0x01, 4, 0, rs232BufferOUT, 1000))
                notify_RASPI_STARTED(sub.q, handlerID, shared->logger);

            shared->logger->log ("esapi::ModuleRasPI => now in RUNNING mode...\n");
            this->stato = ModuleRasPI::eStato::running;
        }
        break;

    case ESAPI_ASK_RASPI_START_FILEUPLOAD:
        //verifico di non avere un file transfer già in corso
        if (NULL != fileUpload.f)
        {
            if (rhea::getTimeNowMSec() - fileUpload.lastTimeUpdatedMSec > 10000)
            {
                rhea::fs::fileClose (fileUpload.f);
                fileUpload.f = NULL;
            }
        }

        if (NULL != fileUpload.f)
        {
            esapi::notify_RASPI_FILEUPLOAD(sub.q, shared->logger, eFileUploadStatus::raspi_fileTransfAlreadyInProgress, (u32)0);
        }
        else
        {
            //provo ad aprire il file in locale
            const u8 *fullFilePathAndName;
            esapi::translate_RASPI_START_FILEUPLOAD (msg, &fullFilePathAndName);
            fileUpload.f = rhea::fs::fileOpenForReadBinary(fullFilePathAndName);
            if (NULL == fileUpload.f)
                esapi::notify_RASPI_FILEUPLOAD(sub.q, shared->logger, eFileUploadStatus::cantOpenSrcFile, (u32)0);
            else
            {
                //chiedo al rasPI se possiamo uppare il file
                //# R [0x03] [fileSizeMSB3] [fileSizeMSB2] [filesizeMSB1] [filesizeLSB] [packetSizeMSB] [packetSizeLSB] [lenFilename] [filename...] [ck]
                fileUpload.totalFileSizeBytes = (u32)rhea::fs::filesize(fileUpload.f);
                fileUpload.bytesSentSoFar = 0;
                fileUpload.packetSizeBytes = 1000;
                    
                //al rasPI mando solo il filename, senza il path
                u8 onlyFilename[128];
                rhea::fs::extractFileNameWithExt (fullFilePathAndName, onlyFilename, sizeof(onlyFilename));
                const u8 onlyFilenameLen = (u8)rhea::string::utf8::lengthInBytes(onlyFilename);

                u32 ct = 0;
                rs232BufferOUT[ct++] = '#';
                rs232BufferOUT[ct++] = 'R';
                rs232BufferOUT[ct++] = 0x03;
                rhea::utils::bufferWriteU32(&rs232BufferOUT[ct], fileUpload.totalFileSizeBytes);
                ct += 4;
                rhea::utils::bufferWriteU16(&rs232BufferOUT[ct], fileUpload.packetSizeBytes);
                ct += 2;
                rs232BufferOUT[ct++] = onlyFilenameLen;
                memcpy (&rs232BufferOUT[ct], onlyFilename, onlyFilenameLen);
                ct += onlyFilenameLen;
    
                rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc(rs232BufferOUT, ct);
                ct++;
                shared->protocol->rs232_write (rs232BufferOUT, ct);

                //attendo risposta
                //# R [0x03] [error] [ck]
                if (!priv_boot_waitAnswer(shared, 'R', 0x03, 5, 0, rs232BufferOUT, 1000))
                {
                    rhea::fs::fileClose (fileUpload.f);
                    fileUpload.f = NULL;
                    esapi::notify_RASPI_FILEUPLOAD(sub.q, shared->logger, eFileUploadStatus::timeout, (u32)0);
                }
                else
                {
                    //ok, si parte
                    esapi::notify_RASPI_FILEUPLOAD(sub.q, shared->logger, (eFileUploadStatus)rs232BufferOUT[3], (u32)0);
                    priv_boot_handleFileUpload(shared, &sub);
                }
            }
        }
        break;

    case ESAPI_ASK_RASPI_UNZIP:
        {
            const u8 *filename;
            const u8 *dstFolder;
            esapi::translate_RASPI_UNZIP(msg, &filename, &dstFolder);
            const u8 len1 = rhea::string::utf8::lengthInBytes(filename);
            const u8 len2 = rhea::string::utf8::lengthInBytes(dstFolder);

            //chiedo al rasPI
            //# R [0x05] [lenFilename] [lenFolder] [filename_terminato_con_0x00] [folderDest_con_0x00] [ck]
            u32 ct = 0;
            rs232BufferOUT[ct++] = '#';
            rs232BufferOUT[ct++] = 'R';
            rs232BufferOUT[ct++] = 0x05;
            rs232BufferOUT[ct++] = len1;
            rs232BufferOUT[ct++] = len2;
            memcpy (&rs232BufferOUT[ct], filename, len1 + 1);
            ct += len1 + 1;
            memcpy (&rs232BufferOUT[ct], dstFolder, len2 + 1);
            ct += len2 + 1;

            rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc(rs232BufferOUT, ct);
            ct++;

            shared->protocol->rs232_write (rs232BufferOUT, ct);

            //aspetto risposta
            //# R [0x05] [success] [ck]
            if (priv_boot_waitAnswer(shared, 'R', 0x05, 5, 0, rs232BufferOUT, 20000))
            {
                if (rs232BufferOUT[3] == 0x01)
                    notify_RASPI_UNZIP(sub.q, shared->logger, true);
                else
                    notify_RASPI_UNZIP(sub.q, shared->logger, false);
            }
            else
                notify_RASPI_UNZIP(sub.q, shared->logger, false);
        }
        break;
    }
}

//*********************************************************
void ModuleRasPI::boot_handleMsg_R_fromRs232 (sShared *shared, sBuffer *b)
{
    //in stato boot, non ci sono messagi 'R' che devo gestire
    DBGBREAK;
    shared->logger->log ("esapi::ModuleRasPI::boot_handleMsg_R_fromRs232() => ERR, invalid #R command [# R 0x%02X]\n", b->buffer[2]);
    b->removeFirstNBytes(1);
}

//*******************************************************
bool ModuleRasPI::priv_boot_waitAnswer(sShared *shared, u8 command, u8 code, u8 fixedMsgLen, u8 whichByteContainsAdditionMsgLen, u8 *answerBuffer, u32 timeoutMSec)
{
	const u64 timeToExitMSec = rhea::getTimeNowMSec() + timeoutMSec;
	u8 ct = 0;
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		u8 ch;
        if (!shared->protocol->rsr232_readSingleByte(&ch))
		{
			rhea::thread::sleepMSec(10);
			continue;
		}
		
		if (ct == 0)
		{
			if (ch == '#')
				answerBuffer[ct++] = ch;
		}
		else if (ct == 1)
		{
			if (ch == command)
				answerBuffer[ct++] = ch;
			else
				ct = 0;
		}
		else if (ct == 2)
		{
			if (ch == code)
				answerBuffer[ct++] = ch;
			else
				ct = 0;
		}
		else
		{
			answerBuffer[ct++] = ch;

            if (0 == whichByteContainsAdditionMsgLen)
            {
                if (ct == fixedMsgLen)
                {
                    if (rhea::utils::simpleChecksum8_calc(answerBuffer, fixedMsgLen - 1) == answerBuffer[fixedMsgLen - 1])
                        return true;
                    ct = 0;
                }
            }
            else
            {
                //questo caso vuol dire che il messaggio è lungo [fixedMsgLen] + quanto indicato dal byte [whichByteContainsAdditionMsgLen]
                if (ct >= whichByteContainsAdditionMsgLen)
                {
                    const u8 totalMsgSize = answerBuffer[whichByteContainsAdditionMsgLen] + fixedMsgLen;
                    if (ct == totalMsgSize)
                    {
                        if (rhea::utils::simpleChecksum8_calc(answerBuffer, totalMsgSize - 1) == answerBuffer[totalMsgSize - 1])
                            return true;
                        ct = 0;
                    }
                }

            }
		}
	}
	return false;
}

//*********************************************************
void ModuleRasPI::priv_boot_handleFileUpload (sShared *shared, sSubscription *sub)
{
    u32 totalKBSentSoFar = 0;
    while (1)
    {
        const u32 bytesLeft = fileUpload.totalFileSizeBytes - fileUpload.bytesSentSoFar;
        if (0 == bytesLeft)
        {
            //fine, tutto ok
            rhea::fs::fileClose (fileUpload.f);
            fileUpload.f = NULL;
            esapi::notify_RASPI_FILEUPLOAD (sub->q, shared->logger, eFileUploadStatus::finished_OK, fileUpload.totalFileSizeBytes/1024);
            return;
        }

        //dimensione del pacchetto
        u32 packetSize;
        if (bytesLeft >= fileUpload.packetSizeBytes)
            packetSize = fileUpload.packetSizeBytes;
        else
            packetSize = bytesLeft;

        //invio un pacchetto
        //# R [0x04] [...] [ck]
        u32 ct = 0;
        rs232BufferOUT[ct++] = '#';
        rs232BufferOUT[ct++] = 'R';
        rs232BufferOUT[ct++] = 0x04;
        rhea::fs::fileRead (fileUpload.f, &rs232BufferOUT[ct], packetSize);
        ct += packetSize;
        rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc(rs232BufferOUT, ct);
        ct++;

        fileUpload.lastTimeUpdatedMSec = rhea::getTimeNowMSec();

        u8 numRetry = 3;
        while (1)
        {
            shared->protocol->rs232_write (rs232BufferOUT, ct);

            //attendo risposta
            //# R [0x04] [accepted] [ck]
            u8 answer[16];
            if (!priv_boot_waitAnswer(shared, 'R', 0x04, 5, 0, answer, 1000))
            {
                rhea::fs::fileClose (fileUpload.f);
                fileUpload.f = NULL;
                esapi::notify_RASPI_FILEUPLOAD(sub->q, shared->logger, eFileUploadStatus::timeout, (u32)0);
                return;
            }

            if (answer[3] != 0x00)
                break;

            //qualcosa è andato male, devo rimandare il pacchetto
            if (numRetry == 0)
            {
                rhea::fs::fileClose (fileUpload.f);
                fileUpload.f = NULL;
                esapi::notify_RASPI_FILEUPLOAD(sub->q, shared->logger, eFileUploadStatus::timeout, (u32)0);
                return;
            }

            --numRetry;
        }

        fileUpload.bytesSentSoFar += packetSize;
        const u32 kbSoFar = fileUpload.bytesSentSoFar / 1024;
        if (kbSoFar != totalKBSentSoFar)
        {
            totalKBSentSoFar = kbSoFar;
            esapi::notify_RASPI_FILEUPLOAD (sub->q, shared->logger, eFileUploadStatus::inProgress, totalKBSentSoFar);
        }
    }
}




/********************************************************************
 *
 *  stato = eStato_runnig
 *
 * In questa fase, il modulo rasPI è master e fa da tunnel tra la websocket che gira sul rasPI e la socket di socketBridge sulla GPU.
 * Tutto quello che rasPI riceve via socket, lo spedisce pari pari via seriale alla GPU che a sua volta lo manda a socketBridge
 */

//*********************************************************
void ModuleRasPI::running_handleMsgFromSubscriber (sShared *shared, sSubscription &sub, const rhea::thread::sMsg &msg, u16 handlerID)
{
    switch (msg.what)
    {
    default:
        shared->logger->log("ModuleRasPI::running_handleMsgFromSubscriber() => invalid msg.what [0x%02X]\n", msg.what);
        break;

    case ESAPI_ASK_GET_MODULE_TYPE_AND_VER:
        notify_MODULE_TYPE_AND_VER (sub.q, handlerID, shared->logger, shared->moduleInfo.type, shared->moduleInfo.verMajor, shared->moduleInfo.verMinor);
        break;
    }
}

//*********************************************************
void ModuleRasPI::running_handleMsg_R_fromRs232	(sShared *shared, sBuffer *b)
{
	const u8 COMMAND_CHAR = 'R';

    while (b->numBytesInBuffer >= 3)
    {
        assert(b->numBytesInBuffer >= 3 && b->buffer[0] == '#' && b->buffer[1] == COMMAND_CHAR);
        const u8 commandCode = b->buffer[2];

        switch (commandCode)
        {
        default:
            shared->logger->log("esapi::ModuleRasPI::RS232 => invalid commandNum [%c][%c]\n", b->buffer[1], commandCode);
            b->removeFirstNBytes(2);
            return;

        case 0x01:
            //new socket connected
            //ricevuto: # R [0x01] [socketUID_MSB3] [socketUID_MSB2] [socketUID_MSB1] [socketUID_LSB] [ck]
            {
                //parse del messaggio
                bool bValidCk = false;
                u32 socketUID = 0;
                const u32 MSG_LEN = esapi::buildMsg_R0x01_newSocket_parse (b->buffer, b->numBytesInBuffer, &bValidCk, &socketUID);
                if (0 == MSG_LEN)
                    return;
                if (!bValidCk)
                {
                    b->removeFirstNBytes(2);
                    return;
                }

                //rimuovo il msg dal buffer
                b->removeFirstNBytes(MSG_LEN);

                //creo una nuova socket e la metto in comunicazione con sokbridge
                sConnectedSocket cl;
                rhea::socket::init (&cl.sok);
                shared->logger->log ("esapi::ModuleRasPI::RS232 => new socket connection...");
                eSocketError err = rhea::socket::openAsTCPClient (&cl.sok, "127.0.0.1", 2280);
                if (err != eSocketError::none)
                {
                    shared->logger->log ("FAIL\n");
                    DBGBREAK;
                    //comunico la disconnessione via seriale
                    const u32 n = esapi::buildMsg_R0x02_closeSocket (socketUID, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
                    shared->protocol->rs232_write (rs232BufferOUT, n);
                }
                else
                {
                    cl.uid = socketUID;
                    sockettList.append(cl);
                    shared->waitableGrp.addSocket (cl.sok, cl.uid);
                    shared->logger->log ("OK, socket id [%d]\n", cl.uid);
                }
            }
            break;

        case 0x02:
            //Socket close
            //ricevuto: # R [0x02] [socketUID_MSB3] [socketUID_MSB2] [socketUID_MSB1] [socketUID_LSB] [ck]
            {
                //parse del messaggio
                bool bValidCk = false;
                u32 socketUID = 0;
                const u32 MSG_LEN = esapi::buildMsg_R0x02_closeSocket_parse (b->buffer, b->numBytesInBuffer, &bValidCk, &socketUID);
                if (0 == MSG_LEN)
                    return;
                if (!bValidCk)
                {
                    b->removeFirstNBytes(2);
                    return;
                }

                //rimuovo il msg dal buffer
                b->removeFirstNBytes(MSG_LEN);

                //elimino il client
                shared->logger->log ("esapi::ModuleRasPI::RS232 => close socket [%d]\n", socketUID);
                sConnectedSocket *cl = priv_2280_findConnectedSocketByUID (socketUID);
                if (cl)
                    priv_2280_onClientDisconnected (shared, cl->sok, socketUID);
            }
            break;

        case 0x03:
            //rasPI comunica via seriale che la socket [client_uid_4bytes] ha ricevuto i dati [data] per un totale di [lenMSB][lenLSB] bytes
            //Io devo a mia volta inviarli a SocketBridge
            //rcv:	# R [0x03] [client_uid_MSB3] [client_uid_MSB2] [client_uid_MSB1] [client_uid_LSB] [lenMSB] [lenLSB] [data…] [ck]
            {
                if (b->numBytesInBuffer < 9)
                    return;

                const u32 uid = rhea::utils::bufferReadU32 (&b->buffer[3]);
                const u32 dataLen = rhea::utils::bufferReadU16(&b->buffer[7]);

                if (b->numBytesInBuffer < 10 + dataLen)
                    return;

                const u8* data = &b->buffer[9];
                const u8 ck = b->buffer[9 + dataLen];
                if (rhea::utils::simpleChecksum8_calc(b->buffer, 9 + dataLen) != ck)
                {
                    b->removeFirstNBytes(2);
                    return;
                }

                //messaggio valido, lo devo mandare via socket al client giusto
                if (dataLen)
                {
                    sConnectedSocket *cl = priv_2280_findConnectedSocketByUID(uid);
                    if (NULL != cl)
                    {
                        rhea::socket::write (cl->sok, data, dataLen);
                        shared->logger->log ("esapi::ModuleRasPI::RS232 => rcv [%d] bytes, sending to socket [%d]\n", dataLen, cl->uid);
                    }
                    else
                    {
                        DBGBREAK;
                    }
                }

                //rimuovo il msg dal buffer di input
                b->removeFirstNBytes(10 + dataLen);
            }
            break;
        }
    }
}

//*********************************************************
void ModuleRasPI::running_handleMsgFromSocket (sShared *shared, OSSocket &sok, u32 uid)
{
    //ho ricevuto un msg da socketBrdige, devo spedirlo al rasPI via seriale
	sConnectedSocket *cl = priv_2280_findConnectedSocketByUID(uid);
	if (NULL == cl)
	{
		DBGBREAK;
		return;
	}
	assert (rhea::socket::compare(cl->sok, sok));


	i32 nBytesLetti = rhea::socket::read (cl->sok, sokBuffer, SIZE_OF_SOKBUFFER, 100);
	if (nBytesLetti == 0)
	{
		//connessione chiusa
		priv_2280_onClientDisconnected(shared, sok, uid);
		return;
	}
	if (nBytesLetti < 0)
	{
		//la chiamata sarebbe stata bloccante, non dovrebbe succedere
        shared->logger->log ("esapi::ModuleRasPI::socket[%d] => read bloccante...\n", cl->uid);
		return;
	}

	//spedisco lungo la seriale
	u32 ct = 0;
	rs232BufferOUT[ct++] = '#';
	rs232BufferOUT[ct++] = 'R';
	rs232BufferOUT[ct++] = 0x04;

	rhea::utils::bufferWriteU32 (&rs232BufferOUT[ct], cl->uid);
	ct += 4;

	rhea::utils::bufferWriteU16 (&rs232BufferOUT[ct], nBytesLetti);
	ct += 2;

	memcpy (&rs232BufferOUT[ct], sokBuffer, nBytesLetti);
	ct += nBytesLetti;

	rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc (rs232BufferOUT, ct);
	ct++;

    assert (ct < SIZE_OF_RS232BUFFEROUT);

	shared->protocol->rs232_write (rs232BufferOUT, ct);
    shared->logger->log ("esapi::ModuleRasPI::socket[%d] => rcv [%d] bytes, sending to rasPI\n", cl->uid, nBytesLetti);

    if (nBytesLetti > 500)
        rhea::thread::sleepMSec(100);
}
