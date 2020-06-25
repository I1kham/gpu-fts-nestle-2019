#include "ESAPIModuleRasPI.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../CPUBridge/CPUBridge.h"

using namespace esapi;

//********************************************************
ModuleRasPI::ModuleRasPI()
{
    fileUpload.f = NULL;
    fileUpload.lastTimeUpdatedMSec = 0;
}

//********************************************************
bool ModuleRasPI::setup ( sGlob *glob)
{
    this->glob = glob;
    this->glob->moduleInfo.type = esapi::eExternalModuleType_rasPI_wifi_REST;

    //buffer
	rs232BufferIN.alloc (glob->localAllocator, SIZE_OF_RS232BUFFERIN);
	rs232BufferOUT = (u8*)RHEAALLOC(glob->localAllocator, SIZE_OF_RS232BUFFEROUT);
    sokBuffer = (u8*)RHEAALLOC(glob->localAllocator, SIZE_OF_SOKBUFFER);

    //aggiungo la serviceMsgQ alla waitList
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(glob->serviceMsgQR, &h);
        waitableGrp.addEvent(h, WAITLIST_EVENT_FROM_SERVICE_MSGQ);
    }

    //aggiungo i subscriber alla wait list
    for (u32 i=0; i<glob->subscriberList.list.getNElem();i++)
    {
        waitableGrp.addEvent(glob->subscriberList.list(i)->hEvent, WAITLIST_EVENT_FROM_A_SUBSCRIBER);
    }

    //socket list
    sockettList.setup (glob->localAllocator, 128);
	return true;
}

//********************************************************
void ModuleRasPI::priv_unsetup()
{
    //rimuovo la serviceMsgQ dalla waitList
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(glob->serviceMsgQR, &h);
        waitableGrp.removeEvent(h);
    }

    rs232BufferIN.free (glob->localAllocator);
    RHEAFREE(glob->localAllocator, rs232BufferOUT);
    RHEAFREE(glob->localAllocator, sokBuffer);
    sockettList.unsetup ();
}

//*********************************************************
void ModuleRasPI::priv_rs232_sendBuffer (const u8 *buffer, u32 numBytesToSend)
{
    u32 nSent = 0;
    while (1)
    {
        const u32 n = rhea::rs232::writeBuffer (glob->com, buffer, numBytesToSend);
        if (n > 0)
        {
            nSent += n;
            if (nSent >= numBytesToSend)
                return;
        }
        else
        {
            DBGBREAK;
        }
        rhea::thread::sleepMSec(2);
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

//********************************************************
eExternalModuleType ModuleRasPI::run()
{
    glob->logger->log ("ModuleRasPI:: now in BOOT mode...\n");
    glob->logger->incIndent();
    priv_boot_run();
    glob->logger->log ("FIN\n");
    glob->logger->decIndent();

    glob->logger->log ("ModuleRasPI:: now in RUNNING mode...\n");
    glob->logger->incIndent();
    priv_running_run();
    glob->logger->log ("FIN\n");
    glob->logger->decIndent();

    priv_unsetup();
    return esapi::eExternalModuleType_none;
}

//*********************************************************
void ModuleRasPI::priv_handleMsgFromServiceQ()
{
	rhea::thread::sMsg msg;
    while (rhea::thread::popMsg (glob->serviceMsgQR, &msg))
    {
        switch (msg.what)
        {
        default:
            DBGBREAK;
            break;

        case ESAPI_SERVICECH_SUBSCRIPTION_REQUEST:
            {
                sSubscription *sub = glob->subscriberList.onSubscriptionRequest (glob->localAllocator, glob->logger, msg);
                waitableGrp.addEvent(sub->hEvent, WAITLIST_EVENT_FROM_A_SUBSCRIBER);
            }
			break;
		}
		
		rhea::thread::deleteMsg(msg);
    }
}


/********************************************************
 * In questa fase, il modulo rasPI è slave, ovvero attende comandi da me via seriale; io a mia volta attendo comandi via subscriber.
 * Questa fase termina quando spedisco il comando # R 0x01 [ck] che manda il modulo rasPI
 * nella modaliù operativa vera e propria.
 * I comandi, io li mando se qualche subscriber mi dice di farlo..
 */
void ModuleRasPI::priv_boot_run()
{
    bQuit = false;
    while (bQuit == false)
    {
		//qui sarebbe bello rimanere in wait per sempre fino a che un evento non scatta oppure la seriale ha dei dati in input.
		//TODO: trovare un modo multipiattaforma per rimanere in wait sulla seriale (in linux è facile, è windows che rogna un po')
        const u8 nEvents = waitableGrp.wait(10);
		for (u8 i = 0; i < nEvents; i++)
		{
			switch (waitableGrp.getEventOrigin(i))
			{
			case OSWaitableGrp::evt_origin_osevent:
				{
                    switch (waitableGrp.getEventUserParamAsU32(i))
                    {
                    case WAITLIST_EVENT_FROM_SERVICE_MSGQ:
                        priv_handleMsgFromServiceQ();
                        break;

                    case WAITLIST_EVENT_FROM_A_SUBSCRIBER:
				        //evento generato dalla msgQ di uno dei miei subscriber
                        {
                            OSEvent h = waitableGrp.getEventSrcAsOSEvent(i);
                            sSubscription *sub = glob->subscriberList.findByOSEvent(h);
						    if (sub)
                                priv_boot_handleMsgFromSubscriber(sub);
				        }
                        break;

                    default:
						DBGBREAK;
                        glob->logger->log("esapi::ModuleRasPI::run() => unhandled OSEVENT from waitGrp [%d]\n", waitableGrp.getEventUserParamAsU32(i));
					}
				}
				break;

			default:
                glob->logger->log("esapi::ModuleRasPI::run() => unhandled event from waitGrp [%d]\n", waitableGrp.getEventOrigin(i));
				break;
			}
		}
    }
}

//*********************************************************
void ModuleRasPI::priv_boot_handleMsgFromSubscriber(sSubscription *sub)
{
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg (sub->q.hFromSubscriberToMeR, &msg))
    {
        const u16 handlerID = (u16)msg.paramU32;

        switch (msg.what)
        {
        default:
            glob->logger->log("ModuleRasPI::priv_boot_handleMsgFromSubscriber() => invalid msg.what [0x%02X]\n", msg.what);
            break;

        case ESAPI_ASK_UNSUBSCRIBE:
            rhea::thread::deleteMsg(msg);
            waitableGrp.removeEvent (sub->hEvent);
            glob->subscriberList.unsubscribe (glob->localAllocator, sub);
            return;

        case ESAPI_ASK_GET_MODULE_TYPE_AND_VER:
            notify_MODULE_TYPE_AND_VER (sub->q, handlerID, glob->logger, glob->moduleInfo.type, glob->moduleInfo.verMajor, glob->moduleInfo.verMinor);
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
                priv_rs232_sendBuffer (rs232BufferOUT, ct);
                if (priv_boot_waitAnswer('R', 0x01, 4, 0, rs232BufferOUT, 1000))
                    notify_RASPI_STARTED(sub->q, handlerID, glob->logger);
                bQuit = true;
            }
            break;

        case ESAPI_ASK_RASPI_GET_IPandSSID:
            //chiedo al rasPI IP e SSID
            {
                u32 ct = 0;
                rs232BufferOUT[ct++] = '#';
                rs232BufferOUT[ct++] = 'R';
                rs232BufferOUT[ct++] = 0x02;
                rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc(rs232BufferOUT, 3);
                ct++;
                priv_rs232_sendBuffer (rs232BufferOUT, ct);

                //la risposta è a lunghezza variabile
                //# R [0x02] [ip1] [ip2] [ip3] [ip4] [lenSSID] [ssidString...] [ck]
                if (priv_boot_waitAnswer('R', 0x02, 9, 8, rs232BufferOUT, 1000))
                {
                    const char *ssid = (const char*)&rs232BufferOUT[8];
                    rs232BufferOUT[8 + rs232BufferOUT[7]] = 0x00;
                    notify_RASPI_WIFI_IPandSSID (sub->q, handlerID, glob->logger, rs232BufferOUT[3], rs232BufferOUT[4], rs232BufferOUT[5], rs232BufferOUT[6], ssid);
                }
            }
            break;

        case ESAPI_ASK_RASPI_START_FILEUPLOAD:
            //verifico di non avere un file transfer già in corso
            if (NULL != fileUpload.f)
            {
                if (rhea::getTimeNowMSec() - fileUpload.lastTimeUpdatedMSec > 10000)
                {
                    fclose (fileUpload.f);
                    fileUpload.f = NULL;
                }
            }

            if (NULL != fileUpload.f)
            {
                esapi::notify_RASPI_FILEUPLOAD(sub->q, glob->logger, eFileUploadStatus_raspi_fileTransfAlreadyInProgress, (u32)0);
            }
            else
            {
                //provo ad aprire il file in locale
                const u8 *fullFilePathAndName;
                esapi::translate_RASPI_START_FILEUPLOAD (msg, &fullFilePathAndName);
                fileUpload.f = rhea::fs::fileOpenForReadBinary(fullFilePathAndName);
                if (NULL == fileUpload.f)
                    esapi::notify_RASPI_FILEUPLOAD(sub->q, glob->logger, eFileUploadStatus_cantOpenSrcFile, (u32)0);
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
                    priv_rs232_sendBuffer (rs232BufferOUT, ct);

                    //attendo risposta
                    //# R [0x03] [error] [ck]
                    if (!priv_boot_waitAnswer('R', 0x03, 5, 0, rs232BufferOUT, 1000))
                    {
                        fclose (fileUpload.f);
                        fileUpload.f = NULL;
                        esapi::notify_RASPI_FILEUPLOAD(sub->q, glob->logger, eFileUploadStatus_timeout, (u32)0);
                    }
                    else
                    {
                        //ok, si parte
                        esapi::notify_RASPI_FILEUPLOAD(sub->q, glob->logger, (eFileUploadStatus)rs232BufferOUT[3], (u32)0);
                        priv_boot_handleFileUpload(sub);
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

                priv_rs232_sendBuffer (rs232BufferOUT, ct);

                //aspetto risposta
                //# R [0x05] [success] [ck]
                if (priv_boot_waitAnswer('R', 0x05, 5, 0, rs232BufferOUT, 20000))
                {
                    if (rs232BufferOUT[3] == 0x01)
                        notify_RASPI_UNZIP(sub->q, glob->logger, true);
                    else
                        notify_RASPI_UNZIP(sub->q, glob->logger, false);
                }
                else
                    notify_RASPI_UNZIP(sub->q, glob->logger, false);

            }
            break;
        }

        rhea::thread::deleteMsg(msg);
    }
}

//*******************************************************
bool ModuleRasPI::priv_boot_waitAnswer(u8 command, u8 code, u8 fixedMsgLen, u8 whichByteContainsAdditionMsgLen, u8 *answerBuffer, u32 timeoutMSec)
{
	const u64 timeToExitMSec = rhea::getTimeNowMSec() + timeoutMSec;
	u8 ct = 0;
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		u8 ch;
		if (!rhea::rs232::readBuffer(glob->com, &ch, 1))
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
void ModuleRasPI::priv_boot_handleFileUpload(sSubscription *sub)
{
    u32 totalKBSentSoFar = 0;
    while (1)
    {
        const u32 bytesLeft = fileUpload.totalFileSizeBytes - fileUpload.bytesSentSoFar;
        if (0 == bytesLeft)
        {
            //fine, tutto ok
            fclose (fileUpload.f);
            fileUpload.f = NULL;
            esapi::notify_RASPI_FILEUPLOAD (sub->q, glob->logger, eFileUploadStatus_finished_OK, fileUpload.totalFileSizeBytes/1024);
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
            priv_rs232_sendBuffer (rs232BufferOUT, ct);

            //attendo risposta
            //# R [0x04] [accepted] [ck]
            u8 answer[16];
            if (!priv_boot_waitAnswer('R', 0x04, 5, 0, answer, 1000))
            {
                fclose (fileUpload.f);
                fileUpload.f = NULL;
                esapi::notify_RASPI_FILEUPLOAD(sub->q, glob->logger, eFileUploadStatus_timeout, (u32)0);
                return;
            }

            if (answer[3] != 0x00)
                break;

            //qualcosa è andato male, devo rimandare il pacchetto
            if (numRetry == 0)
            {
                fclose (fileUpload.f);
                fileUpload.f = NULL;
                esapi::notify_RASPI_FILEUPLOAD(sub->q, glob->logger, eFileUploadStatus_timeout, (u32)0);
                return;
            }

            --numRetry;
        }

        fileUpload.bytesSentSoFar += packetSize;
        const u32 kbSoFar = fileUpload.bytesSentSoFar / 1024;
        if (kbSoFar != totalKBSentSoFar)
        {
            totalKBSentSoFar = kbSoFar;
            esapi::notify_RASPI_FILEUPLOAD (sub->q, glob->logger, eFileUploadStatus_inProgress, totalKBSentSoFar);
        }
    }
}


/********************************************************
 * In questa fase, il modulo rasPI è master e fa da tunnel tra la websocket che gira sul rasPI e la socket
 * di socketBridge sulla GPU.
 * Tutto quello che rasPI riceve via socket, lo spedisce pari pari via seriale alla GPU che a sua volta lo
 * manda a socketBridge
 */
void ModuleRasPI::priv_running_run()
{
    bQuit = false;
    while (bQuit == false)
    {
		//qui sarebbe bello rimanere in wait per sempre fino a che un evento non scatta oppure la seriale ha dei dati in input.
		//TODO: trovare un modo multipiattaforma per rimanere in wait sulla seriale (in linux è facile, è windows che rogna un po')
        const u8 nEvents = waitableGrp.wait(10);
		for (u8 i = 0; i < nEvents; i++)
		{
			switch (waitableGrp.getEventOrigin(i))
			{
			case OSWaitableGrp::evt_origin_osevent:
				{
                    switch (waitableGrp.getEventUserParamAsU32(i))
                    {
                    case WAITLIST_EVENT_FROM_SERVICE_MSGQ:
                        priv_handleMsgFromServiceQ();
                        break;

                    case WAITLIST_EVENT_FROM_A_SUBSCRIBER:
				        //evento generato dalla msgQ di uno dei miei subscriber
                        {
                            OSEvent h = waitableGrp.getEventSrcAsOSEvent(i);
                            sSubscription *sub = glob->subscriberList.findByOSEvent(h);
						    if (sub)
                                priv_running_handleMsgFromSubscriber(sub);
				        }
                        break;

                    default:
						DBGBREAK;
                        glob->logger->log("esapi::ModuleRasPI::run() => unhandled OSEVENT from waitGrp [%d]\n", waitableGrp.getEventUserParamAsU32(i));
					}
				}
				break;

			case OSWaitableGrp::evt_origin_socket:
				{
					//Ho ricevuto dei dati lungo la socket, devo spedirli via seriale al rasPI
					const u32 clientUID = waitableGrp.getEventUserParamAsU32(i);
					OSSocket sok = waitableGrp.getEventSrcAsOSSocket (i);
					priv_2280_sendDataViaRS232 (sok, clientUID);
				}
				break;

			default:
                glob->logger->log("esapi::ModuleRasPI::run() => unhandled event from waitGrp [%d]\n", waitableGrp.getEventOrigin(i));
				break;
			}
		}

        //gestione comunicazione seriale
        priv_running_handleRS232(rs232BufferIN);
    }
}

//*********************************************************
void ModuleRasPI::priv_running_handleMsgFromSubscriber(sSubscription *sub)
{
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg (sub->q.hFromSubscriberToMeR, &msg))
    {
        const u16 handlerID = (u16)msg.paramU32;

        switch (msg.what)
        {
        default:
            glob->logger->log("ModuleRasPI::priv_running_handleMsgFromSubscriber() => invalid msg.what [0x%02X]\n", msg.what);
            break;

        case ESAPI_ASK_UNSUBSCRIBE:
            rhea::thread::deleteMsg(msg);
            waitableGrp.removeEvent (sub->hEvent);
            glob->subscriberList.unsubscribe (glob->localAllocator, sub);
            return;

        case ESAPI_ASK_GET_MODULE_TYPE_AND_VER:
            notify_MODULE_TYPE_AND_VER (sub->q, handlerID, glob->logger, esapi::eExternalModuleType_rasPI_wifi_REST, glob->moduleInfo.verMajor, glob->moduleInfo.verMinor);
            break;

        case ESAPI_ASK_RASPI_GET_IPandSSID:
            //chiedo al rasPI IP e SSID
            {
                u32 ct = 0;
                rs232BufferOUT[ct++] = '#';
                rs232BufferOUT[ct++] = 'R';
                rs232BufferOUT[ct++] = 0x02;
                rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc(rs232BufferOUT, ct);
                ct++;
                priv_rs232_sendBuffer (rs232BufferOUT, ct);

                //la risposta è a lunghezza variabile
                //# R [0x02] [ip1] [ip2] [ip3] [ip4] [lenSSID] [ssidString...] [ck]
                if (priv_boot_waitAnswer('R', 0x02, 9, 8, rs232BufferOUT, 1000))
                {
                    const char *ssid = (const char*)&rs232BufferOUT[8];
                    rs232BufferOUT[8 + rs232BufferOUT[7]] = 0x00;
                    notify_RASPI_WIFI_IPandSSID (sub->q, handlerID, glob->logger, rs232BufferOUT[3], rs232BufferOUT[4], rs232BufferOUT[5], rs232BufferOUT[6], ssid);
                }
            }
            break;
        }
        rhea::thread::deleteMsg(msg);
    }
}

//*********************************************************
void ModuleRasPI::priv_running_handleRS232 (sBuffer &b)
{
    while (1)
    {
	    //leggo tutto quello che posso dalla seriale e bufferizzo in [b]
        const u16 nBytesAvailInBuffer = (u16)(b.SIZE - b.numBytesInBuffer);

		if (0 == nBytesAvailInBuffer)
        {
			DBGBREAK;
        }

	    if (nBytesAvailInBuffer > 0)
	    {
		    const u32 nRead = rhea::rs232::readBuffer(glob->com, &b.buffer[b.numBytesInBuffer], nBytesAvailInBuffer);
		    b.numBytesInBuffer += (u16)nRead;
	    }
    
		if (0 == b.numBytesInBuffer)
			return;

        //provo ad estrarre un msg 'raw' dal mio buffer.
        //Cerco il carattere di inizio buffer ed eventualmente butto via tutto quello che c'è prima
        u32 i = 0;
        while (i < b.numBytesInBuffer && b.buffer[i] != (u8)'#')
            i++;

        if (b.buffer[i] != (u8)'#')
        {
            b.reset();
            return;
        }

        b.removeFirstNBytes(i);
        assert (b.buffer[0] == (u8)'#');
        i = 0;

        if (b.numBytesInBuffer < 3)
            return;

        const u8 commandChar = b.buffer[1];
        switch (commandChar)
        {
        default:
            glob->logger->log ("esapi::ModuleRasPI::priv_running_handleRS232() => invalid command char [%c]\n", commandChar);
            b.removeFirstNBytes(1);
            break;

		case 'R':   if (!priv_running_handleCommand_R (b)) return;    break;
			break;
        }
    } //while(1)
}

/*********************************************************
 * ritorna true se ha consumato qualche byte di buffer.
 * ritorna false altrimenti. In questo caso, vuol dire che i byte in buffer sembravano essere un valido messaggio ma probabilmente manca ancora qualche
 *  bytes al completamento del messaggio stesso. Bisogna quindi attendere che ulteriori byte vengano appesi al buffer
 */
bool ModuleRasPI::priv_running_handleCommand_R (sBuffer &b)
{
	const u8 COMMAND_CHAR = 'R';

    assert(b.numBytesInBuffer >= 3 && b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
	const u8 commandCode = b.buffer[2];

	switch (commandCode)
	{
	default:
		glob->logger->log("esapi::ModuleRasPI => invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
		b.removeFirstNBytes(2);
		return true;

	case 0x01:
		//ho ricevuto un R 0x01 new socket connected
		{
            //parse del messaggio
            bool bValidCk = false;
            u32 socketUID = 0;
            const u32 MSG_LEN = esapi::buildMsg_R0x01_newSocket_parse (b.buffer, b.numBytesInBuffer, &bValidCk, &socketUID);
            if (0 == MSG_LEN)
                return false;
            if (!bValidCk)
            {
                b.removeFirstNBytes(2);
                return true;
            }

            //rimuovo il msg dal buffer
            b.removeFirstNBytes(MSG_LEN);

			//creo una nuova socket e la metto in comunicazione con sokbridge
			sConnectedSocket cl;
			rhea::socket::init (&cl.sok);
			glob->logger->log ("esapi::ModuleRasPI => new socket connection...");
			eSocketError err = rhea::socket::openAsTCPClient (&cl.sok, "127.0.0.1", 2280);
			if (err != eSocketError_none)
			{
				glob->logger->log ("FAIL\n");
				DBGBREAK;
				//comunico la disconnessione via seriale
                const u32 n = esapi::buildMsg_R0x02_closeSocket (socketUID, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
			    priv_rs232_sendBuffer (rs232BufferOUT, n);
			}
			else
			{
				cl.uid = socketUID;
				sockettList.append(cl);
				waitableGrp.addSocket (cl.sok, cl.uid);
				glob->logger->log ("OK, socket id [%d]\n", cl.uid);
			}
			return true;
		}
		break;

	case 0x02:
		//ho ricevuto un R 0x02 socket close
		{
            //parse del messaggio
            bool bValidCk = false;
            u32 socketUID = 0;
            const u32 MSG_LEN = esapi::buildMsg_R0x02_closeSocket_parse (b.buffer, b.numBytesInBuffer, &bValidCk, &socketUID);
            if (0 == MSG_LEN)
                return false;
            if (!bValidCk)
            {
                b.removeFirstNBytes(2);
                return true;
            }

            //rimuovo il msg dal buffer
            b.removeFirstNBytes(MSG_LEN);

			//elimino il client
			sConnectedSocket *cl = priv_2280_findConnectedSocketByUID (socketUID);
			if (cl)
				priv_2280_onClientDisconnected (cl->sok, socketUID);
		}
		return true;

	case 0x03:
		//rasPIESAPI: rasPI comunica via seriale che la socket [client_uid_4bytes] ha ricevuto i dati  [data] per un totale di [lenMSB][lenLSB] bytes
		//rcv:	# R [0x03] [client_uid_MSB3] [client_uid_MSB2] [client_uid_MSB1] [client_uid_LSB] [lenMSB] [lenLSB] [data…] [ck]
		{
			if (b.numBytesInBuffer < 9)
				return false;

			const u32 uid = rhea::utils::bufferReadU32 (&b.buffer[3]);
            const u32 dataLen = rhea::utils::bufferReadU16(&b.buffer[7]);

            if (b.numBytesInBuffer < 10 + dataLen)
				return false;

			const u8* data = &b.buffer[9];
			const u8 ck = b.buffer[9 + dataLen];
			if (rhea::utils::simpleChecksum8_calc(b.buffer, 9 + dataLen) != ck)
			{
				b.removeFirstNBytes(2);
				return true;
			}

			//messaggio valido, lo devo mandare via socket al client giusto
			if (dataLen)
			{
				sConnectedSocket *cl = priv_2280_findConnectedSocketByUID(uid);
				if (NULL != cl)
				{
					rhea::socket::write (cl->sok, data, dataLen);
					glob->logger->log ("rcv [%d] bytes from RS232, sending to socket [%d]\n", dataLen, cl->uid);
				}
				else
				{
					DBGBREAK;
				}
			}

			//rimuovo il msg dal buffer di input
			b.removeFirstNBytes(10 + dataLen);
			return true;
		}
		break;
	}
}

//***************************************************************
void ModuleRasPI::priv_2280_onClientDisconnected (OSSocket &sok, u32 uid)
{
	for (u32 i = 0; i < sockettList.getNElem(); i++)
	{
		if (sockettList(i).uid == uid)
		{
			assert (rhea::socket::compare(sockettList(i).sok, sok));

			waitableGrp.removeSocket (sockettList[i].sok);
			rhea::socket::close(sockettList[i].sok);
			sockettList.removeAndSwapWithLast(i);

			//comunico la disconnessione via seriale
            const u32 n = esapi::buildMsg_R0x02_closeSocket (uid, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
			priv_rs232_sendBuffer (rs232BufferOUT, n);
			glob->logger->log ("esapi::ModuleRasPI => socket [%d] disconnected\n", uid);
			return;
		}
	}

	//non ho trovato il client nella lista...
	DBGBREAK;
}

//***************************************************************
void ModuleRasPI::priv_2280_sendDataViaRS232 (OSSocket &sok, u32 uid)
{
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
		priv_2280_onClientDisconnected(sok, uid);
		return;
	}
	if (nBytesLetti < 0)
	{
		//la chiamata sarebbe stata bloccante, non dovrebbe succedere
		glob->logger->log ("esapi::ModuleRasPI => WARN: socket [%d], read bloccante...\n", cl->uid);
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

	priv_rs232_sendBuffer (rs232BufferOUT, ct);
	glob->logger->log ("esapi::ModuleRasPI => rcv [%d] bytes from socket [%d], sending to rasPI\n", nBytesLetti, cl->uid);

    if (nBytesLetti > 500)
        rhea::thread::sleepMSec(100);
}
