#include "raspiCore.h"
#include "../rheaExternalSerialAPI/ESAPI.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/compress/rheaCompress.h"

using namespace raspi;

//*******************************************************
Core::Core ()
{
	localAllocator = NULL;
	logger = &nullLogger;
	sok2280NextID = 0x00;
	rs232BufferOUT = NULL;
	rhea::rs232::setInvalid (com);
	rhea::socket::init (&sok2280);
	fileUpload.f = NULL;
	fileUpload.lastTimeRcvMSec = 0;
}

//***************************************************
void Core::useLogger (rhea::ISimpleLogger *loggerIN)
{
	if (NULL == loggerIN)
		logger = &nullLogger;
	else
		logger = loggerIN;
}

//*******************************************************
bool Core::open (const char *serialPort)
{
    //se non esiste già, creo la cartella temp
    {
        u8 s[512];
        sprintf_s ((char*)s, sizeof(s), "%s/temp", rhea::getPhysicalPathToAppFolder());
        rhea::fs::folderCreate (s);
        rhea::fs::deleteAllFileInFolderRecursively (s, false);
    }

	logger->log ("opening com=%s   ", serialPort);
	if (!rhea::rs232::open(&com, serialPort, eRS232BaudRate_115200, false, false, eRS232DataBits_8, eRS232Parity_No, eRS232StopBits_One, eRS232FlowControl_No, false))
	{
		logger->log ("FAILED. unable to open port [%s]\n", serialPort);
		logger->decIndent();
		return false;
	}
	logger->log ("OK\n");
	
	//buffer vari
	localAllocator = RHEANEW(rhea::getSysHeapAllocator(), rhea::AllocatorSimpleWithMemTrack) ("raspiCore");
	rs232BufferOUT = (u8*)RHEAALLOC(localAllocator, SIZE_OF_RS232BUFFEROUT);
	rs232BufferIN.alloc (localAllocator, 4096);
	sok2280Buffer = (u8*)RHEAALLOC(localAllocator, SIZE_OF_RS232BUFFERIN);
	clientList.setup (localAllocator, 128);

	//recupero il mio IP di rete wifi
    memset (wifiIP, 0, sizeof(wifiIP));
	{
		u32 n = 0;
		sNetworkAdapterInfo *ipList = rhea::netaddr::getListOfAllNerworkAdpaterIPAndNetmask (rhea::getScrapAllocator(), &n);
		if (n)
		{
			for (u32 i = 0; i < n; i++)
			{
				if (strcasecmp (ipList[i].name, "wlan0") == 0)
				{
					rhea::netaddr::ipstrTo4bytes (ipList[i].ip, &wifiIP[0], &wifiIP[1], &wifiIP[2], &wifiIP[3]);
					break;
				}
			}
			
			if (wifiIP[0] == 0)
			{
				//questo è per la versione win, che non ha wlan0
				rhea::netaddr::ipstrTo4bytes (ipList[0].ip, &wifiIP[0], &wifiIP[1], &wifiIP[2], &wifiIP[3]);
			}

			RHEAFREE(rhea::getScrapAllocator(), ipList);
            logger->log ("WIFI IP: %d.%d.%d.%d\n", wifiIP[0], wifiIP[1], wifiIP[2], wifiIP[3]);
		}
	}

    //recupero SSID dell'hotspot
    //Uno script python parte allo startup del rasPI e crea un file di testo di nome "hotspotname.txt" che contiene il nome dell'hotspot
    memset (ssid, 0, sizeof(ssid));
    {
        u8 s[128];
        sprintf_s ((char*)s, sizeof(s), "%s/hotspotname.txt", rhea::getPhysicalPathToAppFolder());
        FILE *f = rhea::fs::fileOpenForReadBinary(s);
        if (NULL == f)
        {
            logger->log ("ERR: unable to open file [%s]\n", s);
            sprintf_s ((char*)ssid, sizeof(ssid), "unknown");
        }
        else
        {
            fread (ssid, sizeof(ssid), 1, f);
            fclose(f);
        }
    }
    logger->log ("Hotspot name:%s\n", ssid);

	return true;
}

//*****************************************************************
void Core::priv_close ()
{
	rhea::rs232::close (com);
	rhea::socket::close (sok2280);

	if (localAllocator)
	{
		RHEAFREE(localAllocator, rs232BufferOUT);
		rs232BufferIN.free (localAllocator);
		RHEAFREE(localAllocator, sok2280Buffer);
		clientList.unsetup();
		RHEADELETE(rhea::getSysHeapAllocator(), localAllocator);
		localAllocator = NULL;
	}
}

/*******************************************************
 *	In questa fase, invio periodicamente alla GPU il comando #A1 per conoscere la versione di API supportata e per capire quando la GPU
 *	diventa disponibile.
 *	Fino a che non ricevo risposta, rimango in questo loop
 */
void Core::priv_identify_run()
{
	reportedESAPIVerMajor = 0;
	reportedESAPIVerMinor = 0;
	reportedGPUType = esapi::eGPUType_unknown;

	u64 timeToSendMsgMSec = 0;
	while (1)
	{
		const u64 timeNowMSec = rhea::getTimeNowMSec();
		if (timeNowMSec >= timeToSendMsgMSec)
		{
			timeToSendMsgMSec = timeNowMSec + 2000;
			logger->log ("requesting API version...\n");

			const u32 nBytesToSend = esapi::buildMsg_A1_getAPIVersion_ask (rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
			priv_rs232_sendBuffer (rs232BufferOUT, nBytesToSend);
			if (priv_identify_waitAnswer('A', '1', 7, rs232BufferOUT, 1000))
			{
				//ho ricevuto risposta valida a comando A 1
				reportedESAPIVerMajor = rs232BufferOUT[3];
				reportedESAPIVerMinor = rs232BufferOUT[4];
				reportedGPUType = (esapi::eGPUType)rs232BufferOUT[5];
				logger->log ("API ver %d.%d, gpuType[%d]\n", reportedESAPIVerMajor, reportedESAPIVerMinor, reportedGPUType);
				break;
			}
		}

		rhea::thread::sleepMSec(100);
	}


	//se arrivo qui, vuol dire che la GPU ha risposto al comando # A 1
	//ora devo comunicare la mia identità e attendere risposta
	while (1)
	{
		const u32 nBytesToSend = esapi::buildMsg_R1_externalModuleIdentify_ask (esapi::eExternalModuleType_rasPI_wifi_REST, VER_MAJOR, VER_MINOR, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
		priv_rs232_sendBuffer (rs232BufferOUT, nBytesToSend);
		if (priv_identify_waitAnswer('R', '1', 5, rs232BufferOUT, 1000))
		{
			//ho ricevuto risposta valida a comando R 1, posso uscire da questa fase
			return;
		}
	}
}

//*******************************************************
bool Core::priv_identify_waitAnswer(u8 command, u8 code, u8 len, u8 *answerBuffer, u32 timeoutMSec)
{
	const u64 timeToExitMSec = rhea::getTimeNowMSec() + timeoutMSec;
	u8 ct = 0;
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		u8 ch;
		if (!rhea::rs232::readBuffer(com, &ch, 1))
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
			if (ct >= len)
			{
				if (rhea::utils::simpleChecksum8_calc(answerBuffer, len-1) == answerBuffer[len-1])
					return true;
				ct = 0;
			}
		}
	}
	return false;
}


/********************************************************
 * In questa fase, il modulo è slave, ovvero attende comandi da GPU via seriale.
 * Questa fase termina alla ricezione del comando # R 0x01 [ck] che manda il modulo
 * nella modaliù operativa vera e propria
 */
void Core::priv_boot_run()
{
    bQuit = false;
    while (bQuit == false)
    {
		priv_boot_rs232_handleCommunication(rs232BufferIN);
		rhea::thread::sleepMSec(10);
    }
}

//*********************************************************
u32 Core::priv_boot_buildMsgBuffer (u8 *buffer, u32 sizeOfBufer, u8 command, const u8 *data, u32 lenOfData)
{
    const u32 totalLenOfMsg = 4 + lenOfData;
    if (sizeOfBufer < totalLenOfMsg)
    {
        DBGBREAK;
        return 0;
    }
    u32 ct = 0;
    buffer[ct++] = '#';
    buffer[ct++] = 'R';
    buffer[ct++] = command;
    if (data && lenOfData)
    {
        memcpy (&buffer[ct], data, lenOfData);
        ct += lenOfData;
    }
    buffer[ct] = rhea::utils::simpleChecksum8_calc (buffer, ct);
    return ct + 1;
}

//*********************************************************
void Core::priv_boot_buildMsgBufferAndSend (u8 *buffer, u32 sizeOfBufer, u8 command, const u8 *data, u32 lenOfData)
{
    const u32 n = priv_boot_buildMsgBuffer (buffer, sizeOfBufer, command, data, lenOfData);
    if (n)
        priv_rs232_sendBuffer (buffer, n);
}

//*********************************************************
void Core::priv_boot_rs232_handleCommunication (sBuffer &b)
{
    while (1)
    {
	    //leggo tutto quello che posso dalla seriale e bufferizzo in [b]
        const u16 nBytesAvailInBuffer = (u16)(b.SIZE - b.numBytesInBuffer);

		if (0 == nBytesAvailInBuffer)
			DBGBREAK;

	    if (nBytesAvailInBuffer > 0)
	    {
		    const u32 nRead = rhea::rs232::readBuffer(com, &b.buffer[b.numBytesInBuffer], nBytesAvailInBuffer);
		    b.numBytesInBuffer += (u16)nRead;
	    }
    
		if (0 == b.numBytesInBuffer)
			return;

        //provo ad estrarre un msg 'raw' dal mio buffer.
        //Cerco il carattere di inizio messaggio (#) ed eventualmente butto via tutto quello che c'è prima
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

        //il ch successivo deve essere 'R'
        if (b.buffer[1] != 'R')
        {
            b.removeFirstNBytes(1);
            continue;
        }

        switch (b.buffer[2])
        {
        default:
            logger->log ("invalid command [%c]\n", b.buffer[2]);
            b.removeFirstNBytes(1);
            break;

        case 0x01:
            // comando start
            // # R [0x01] [ck]
            if (b.numBytesInBuffer < 4)
                return;
            if (!esapi::isValidChecksum (b.buffer[3], b.buffer, 3))
            {
                b.removeFirstNBytes(1);
                break;
            }
            bQuit = true;

            //rimuovo il msg dal buffer
            b.removeFirstNBytes(4);

            //rispondo # R [0x01] [ck]
            priv_boot_buildMsgBufferAndSend (rs232BufferOUT, SIZE_OF_RS232BUFFEROUT, 0x01, NULL, 0);
            return;

        case 0x02: 
            //richiesta IP e SSID
            //# R [0x02] [ck]
            if (b.numBytesInBuffer < 4)
                return;
            if (!esapi::isValidChecksum (b.buffer[3], b.buffer, 3))
            {
                b.removeFirstNBytes(1);
                break;
            }

            //rimuovo il msg dal buffer
            b.removeFirstNBytes(4);

            //rispondo # R [0x02] [ip1] [ip2] [ip3] [ip4] [lenSSID] [ssidString...] [ck]
            {
                const u8 lenSSID = (u8)rhea::string::utf8::lengthInBytes(ssid);
                u32 ct = 0;
                rs232BufferOUT[ct++] = '#';
                rs232BufferOUT[ct++] = 'R';
                rs232BufferOUT[ct++] = 0x02;
                rs232BufferOUT[ct++] = wifiIP[0];
                rs232BufferOUT[ct++] = wifiIP[1];
                rs232BufferOUT[ct++] = wifiIP[2];
                rs232BufferOUT[ct++] = wifiIP[3];
                rs232BufferOUT[ct++] = lenSSID;
                memcpy (&rs232BufferOUT[ct], ssid, lenSSID);
                ct += lenSSID;

                rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc (rs232BufferOUT, ct);
                ct++;
                priv_rs232_sendBuffer (rs232BufferOUT, ct);
            }
            break;

        case 0x03:
            //richiesta inizio upload file
            //rcv:      # R [0x03] [fileSizeMSB3] [fileSizeMSB2] [filesizeMSB1] [filesizeLSB] [packetSizeMSB] [packetSizeLSB] [lenFilename] [filename...] [ck]
            //answer:   # R [0x03] [error] [ck]
            if (b.numBytesInBuffer < 10)
            {
                return;
            }
            else
            {
                const u32 filesizeBytes = rhea::utils::bufferReadU32 (&b.buffer[3]);
                const u16 packetSizeBytes = rhea::utils::bufferReadU16 (&b.buffer[7]);
                const u8 filenameLen = b.buffer[9];
                const u32 totalMsgLen = 11 + filenameLen;
                if (b.numBytesInBuffer < totalMsgLen)
                    return;
                if (!esapi::isValidChecksum (b.buffer[totalMsgLen - 1], b.buffer, totalMsgLen - 1))
                {
                    b.removeFirstNBytes(1);
                    break;
                }

                const u8 *filename = &b.buffer[10];
                b.buffer[totalMsgLen - 1] = 0x00;
				logger->log ("rcv: file upload [%s]\n", filename);

                //non devo avere altri upload in corso
				if (NULL != fileUpload.f)
				{
					if (rhea::getTimeNowMSec() - fileUpload.lastTimeRcvMSec > 5000)
					{
						fclose(fileUpload.f);
						fileUpload.f = NULL;
					}
				}

                if (NULL != fileUpload.f)
                {
					//rispondo con errore
					logger->log ("ERR: file transfer already in progress\n");
					const u8 error = (u8)esapi::eFileUploadStatus_raspi_fileTransfAlreadyInProgress;
					priv_boot_buildMsgBufferAndSend (rs232BufferOUT, SIZE_OF_RS232BUFFEROUT, 0x03, &error, 1);
                }
                else
                {
                    //provo a creare il file nella cartella temp
                    u8 s[512];
                    sprintf_s ((char*)s, sizeof(s), "%s/temp/%s", rhea::getPhysicalPathToAppFolder(), filename);
                    fileUpload.f = rhea::fs::fileOpenForWriteBinary (s);
                    if (NULL == fileUpload.f)
                    {
                        //rispondo con errore
						logger->log ("ERR: cant' open file [%s]\n", s);
                        const u8 error = (u8)esapi::eFileUploadStatus_raspi_cantCreateFileInTempFolder;
                        priv_boot_buildMsgBufferAndSend (rs232BufferOUT, SIZE_OF_RS232BUFFEROUT, 0x03, &error, 1);
                    }
                    else
                    {
						logger->log ("accepted\n", s);
                        //ok, possiamo procedere
                        fileUpload.totalFileSizeBytes = filesizeBytes;
                        fileUpload.packetSizeBytes = packetSizeBytes;
                        fileUpload.rcvBytesSoFar = 0;
						fileUpload.lastTimeRcvMSec = rhea::getTimeNowMSec();

                        const u8 error = 0x00;
                        priv_boot_buildMsgBufferAndSend (rs232BufferOUT, SIZE_OF_RS232BUFFEROUT, 0x03, &error, 1);
                    }
                }

                //rimuovo il msg dal buffer
                b.removeFirstNBytes(totalMsgLen);
            }
            break;

        case 0x04:
            //file upload packet
            //rcv:  # R [0x04] [...] [ck]
            //answ: # R [0x04] [accepted] [ck]
            if (NULL == fileUpload.f)
            {
                b.removeFirstNBytes(1);
                DBGBREAK
            }
            else
            {
                u16 expecxtedPacketLength = fileUpload.packetSizeBytes;
                if (fileUpload.rcvBytesSoFar + fileUpload.packetSizeBytes > fileUpload.totalFileSizeBytes)
                    expecxtedPacketLength = (fileUpload.totalFileSizeBytes - fileUpload.rcvBytesSoFar);

                const u16 expectedMsgLen = 4 + expecxtedPacketLength;
                if (b.numBytesInBuffer < expectedMsgLen)
                    return;
				
				fileUpload.lastTimeRcvMSec = rhea::getTimeNowMSec();
                if (!esapi::isValidChecksum (b.buffer[expectedMsgLen-1], b.buffer, expectedMsgLen-1))
                {
                    //rispondo KO
					logger->log ("packet refused, kbSoFar[%d]\n", fileUpload.rcvBytesSoFar);
                    const u8 accepted = 0;
                    priv_boot_buildMsgBufferAndSend (rs232BufferOUT, SIZE_OF_RS232BUFFEROUT, 0x04, &accepted, 1);
                    b.removeFirstNBytes(1);
                    break;
                }

                rhea::fs::fileWrite (fileUpload.f, &b.buffer[3], expecxtedPacketLength);
                fileUpload.rcvBytesSoFar += expecxtedPacketLength;

                //rimuovo il messaggio dal buffer
                b.removeFirstNBytes(expectedMsgLen);

                //rispondo ok
                const u8 accepted = 1;
                priv_boot_buildMsgBufferAndSend (rs232BufferOUT, SIZE_OF_RS232BUFFEROUT, 0x04, &accepted, 1);

                if (fileUpload.rcvBytesSoFar >= fileUpload.totalFileSizeBytes)
                {
                    fclose(fileUpload.f);
                    fileUpload.f = NULL;
					logger->log ("file transfer finished, kbSoFar[%d]\n", fileUpload.rcvBytesSoFar);
                }
				else
				{
					logger->log ("rcv packet, kbSoFar[%d]\n", fileUpload.rcvBytesSoFar);
				}
			}
            break;

		case 0x05:
			//richiesta di unzippare un file che ho in /temp
			//# R [0x05] [lenFilename] [lenFolder] [filename_terminato_con_0x00] [folderDest_con_0x00] [ck]
            if (b.numBytesInBuffer < 6)
                return;
			else
			{
				const u8 len1 = b.buffer[3];
				const u8 len2 = b.buffer[4];
				const u32 totalLenOfMsg = 6 + (len1 + 1) + (len2 + 1);
				if (b.numBytesInBuffer < totalLenOfMsg)
					return;

				if (!esapi::isValidChecksum (b.buffer[totalLenOfMsg - 1], b.buffer, totalLenOfMsg - 1))
				{
					b.removeFirstNBytes(1);
					break;
				}

				const u8 *filename = &b.buffer[5];
				const u8 *dstFolder = &b.buffer[5+ (len1+1)];
				logger->log ("unzip [%s] [%s]\n", filename, dstFolder);

				u8 src[512];
				u8 dst[512];
				if (rhea::string::utf8::areEqual(dstFolder, (const u8*)"@GUITS", true))
				{
#ifdef LINUX
#ifdef PLATFORM_UBUNTU_DESKTOP
					//unzippo in temp/filenameSenzaExt/
					rhea::fs::extractFileNameWithoutExt (filename, src, sizeof(src));
					sprintf_s ((char*)dst, sizeof(dst), "%s/temp/%s", rhea::getPhysicalPathToAppFolder(), src);
#else
					sprintf_s ((char*)dst, sizeof(dst), "/var/www/html/rhea/GUITS");
#endif
#else
					//unzippo in temp/filenameSenzaExt/
					rhea::fs::extractFileNameWithoutExt (filename, src, sizeof(src));
					sprintf_s ((char*)dst, sizeof(dst), "%s/temp/%s", rhea::getPhysicalPathToAppFolder(), src);
#endif
				}
				else
					sprintf_s ((char*)dst, sizeof(dst), "%s", dstFolder);
				

				sprintf_s ((char*)src, sizeof(src), "%s/temp/%s", rhea::getPhysicalPathToAppFolder(), filename);
				logger->log ("unzipping [%s] into [%s]\n", src, dst);
				bool zipResult = rhea::CompressUtility::decompresAll (src, dst);

				//rimuovo il msg dal buffer
				b.removeFirstNBytes(totalLenOfMsg);

				//rispondo # R [0x05] [success] [ck]
				u8 result = 0x00;
				if (zipResult)
					result = 0x01;
				priv_boot_buildMsgBufferAndSend (rs232BufferOUT, SIZE_OF_RS232BUFFEROUT, 0x05, &result, 1);
				logger->log ("unzip resul [%d]\n", result);
			}
			break;
        }

    } //while(1)

}





//*******************************************************
void Core::run()
{
	logger->log ("Entering IDENITFY mode...\n");
	logger->incIndent();
	priv_identify_run();
	logger->decIndent();


	logger->log ("Entering BOOT mode...\n");
	logger->incIndent();
	priv_boot_run();
	logger->decIndent();

	logger->log ("\n\nEntering RUNNING mode...\n");
	logger->incIndent();
	{
		//socekt in listen sulla 2280
		logger->log ("opening socket on 2280...");
		eSocketError err = rhea::socket::openAsTCPServer(&sok2280, 2280);
		if (err != eSocketError_none)
		{
			logger->log ("ERR code[%d]\n", err);
			logger->log("\n");
		}
		else
			logger->log("OK\n");

		rhea::socket::setReadTimeoutMSec(sok2280, 0);
		rhea::socket::setWriteTimeoutMSec(sok2280, 10000);

		logger->log("listen... ");
		if (!rhea::socket::listen(sok2280))
		{
			logger->log("FAIL\n", err);
			logger->decIndent();
			rhea::socket::close(sok2280);
		}
		else
			logger->log("OK\n");


		//aggiungo la socket al gruppo di oggetti in osservazione
		waitableGrp.addSocket(sok2280, WAITGRP_SOCKET2280);
	}
	logger->decIndent();


	bQuit = false;
	while (bQuit == false)
	{
		const u8 nEvents = waitableGrp.wait(100);
		for (u8 i = 0; i < nEvents; i++)
		{
			if (waitableGrp.getEventOrigin(i) == OSWaitableGrp::evt_origin_socket)
			{
				if (waitableGrp.getEventUserParamAsU32(i) == WAITGRP_SOCKET2280)
				{
					//evento generato dalla socket in listen sulla 2280
					priv_2280_accept();
				}
				else
				{
					//altimenti la socket che si è svegliata deve essere una dei miei client già connessi
					const u32 clientUID = waitableGrp.getEventUserParamAsU32(i);
					OSSocket sok = waitableGrp.getEventSrcAsOSSocket (i);
					priv_2280_onIncomingData (sok, clientUID);
				}
			}
		}

		priv_rs232_handleIncomingData(rs232BufferIN);
	}
}

//*********************************************************
void Core::priv_rs232_sendBuffer (const u8 *buffer, u32 numBytesToSend)
{
	rhea::rs232::writeBuffer (com, buffer, numBytesToSend);
}

//*********************************************************
void Core::priv_rs232_handleIncomingData (sBuffer &b)
{
	while (1)
	{
		//leggo tutto quello che posso dalla seriale e bufferizzo in [b]
		const u16 nBytesAvailInBuffer = (u16)(b.SIZE - b.numBytesInBuffer);
		if (nBytesAvailInBuffer > 0)
		{
			const u32 nRead = rhea::rs232::readBuffer(com, &b.buffer[b.numBytesInBuffer], nBytesAvailInBuffer);
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
			logger->log ("invalid command char [%c]\n", commandChar);
			b.removeFirstNBytes(1);
			break;

		case 'A':   if (!priv_rs232_handleCommand_A (b)) return;    break;
		case 'R':   if (!priv_rs232_handleCommand_R (b)) return;    break;
		}

	} //while(1)

}

/*********************************************************
 * ritorna true se ha consumato qualche byte di buffer.
 * ritorna false altrimenti. In questo caso, vuol dire che i byte in buffer sembravano essere un valido messaggio ma probabilmente manca ancora qualche
 *  bytes al completamento del messaggio stesso. Bisogna quindi attendere che ulteriori byte vengano appesi al buffer
 */
bool Core::priv_rs232_handleCommand_A (sBuffer &b)
{
    const u8 COMMAND_CHAR = 'A';

    assert (b.numBytesInBuffer >= 3 && b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
    const u8 commandCode = b.buffer[2];

    switch (commandCode)
    {
    default:
        logger->log ("invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
        b.removeFirstNBytes(2);
        return true;

    case '1': 
        //risposta di GPU alla mia richiesta di API Version (A1)
		{
			bool bValidCk = false;
			if (!esapi::buildMsg_A1_getAPIVersion_parseResp (b.buffer, b.numBytesInBuffer, &bValidCk, &this->reportedESAPIVerMajor, &this->reportedESAPIVerMinor, &reportedGPUType))
				return false;

			if (!bValidCk)
			{
				b.removeFirstNBytes(2);
				return true;
			}

			logger->log ("reported ESAPI version [%d].[%d], GPUType[%d]\n", reportedESAPIVerMajor, reportedESAPIVerMinor, reportedGPUType);

			//rimuovo il msg dal buffer di input
			b.removeFirstNBytes(7);
		}
		return true;
        break;
    }
}

/********************************************************
 * ritorna true se ha consumato qualche byte di buffer.
 * ritorna false altrimenti. In questo caso, vuol dire che i byte in buffer sembravano essere un valido messaggio ma probabilmente manca ancora qualche
 *  bytes al completamento del messaggio stesso. Bisogna quindi attendere che ulteriori byte vengano appesi al buffer
 */
bool Core::priv_rs232_handleCommand_R (Core::sBuffer &b)
{
	const u8 COMMAND_CHAR = 'R';

	assert(b.numBytesInBuffer >= 3 && b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
	const u8 commandCode = b.buffer[2];

	switch (commandCode)
	{
	default:
		logger->log("invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
		b.removeFirstNBytes(2);
		return true;

	case 0x02:
		//GPU mi comunica che la socket xxx è stata chiusa
		{
           //parse del messaggio
            bool bValidCk = false;
			u32 socketUID;
            const u32 MSG_LEN = esapi::buildMsg_R0x02_closeSocket_parse (b.buffer, b.numBytesInBuffer, &bValidCk, &socketUID);
            if (0 == MSG_LEN)
                return false;
            if (!bValidCk)
            {
				DBGBREAK;
                b.removeFirstNBytes(2);
                return true;
            }
        
            //rimuovo msg dal buffer
            b.removeFirstNBytes(MSG_LEN);

			//elimino il client
			sConnectedSocket *cl = priv_2280_findClientByUID (socketUID);
			if (cl)
				priv_2280_onClientDisconnected (cl->sok, socketUID);
		}
		return true;

	case 0x04:
		//GPU mi sta comunicando dei dati che io devo mandare lungo la socket indicata
		//rcv:   # R [0x04] [client_uid_MSB3] [client_uid_MSB2] [client_uid_MSB1] [client_uid_LSB] [lenMSB] [lenLSB] [data…] [ck]
		{
			if (b.numBytesInBuffer < 9)
				return false;
			
			const u32 uid = rhea::utils::bufferReadU32 (&b.buffer[3]);
			const u16 dataLen = rhea::utils::bufferReadU16(&b.buffer[7]);
			
			if (b.numBytesInBuffer < 10 + dataLen)
				return false;

			const u8* data = &b.buffer[9];
			const u8 ck = b.buffer[9 + dataLen];
			if (rhea::utils::simpleChecksum8_calc(b.buffer, 9 + dataLen) != ck)
			{
				DBGBREAK;
				b.removeFirstNBytes(2);
				return true;
			}

			//messaggio valido, lo devo mandare via socket al client giusto
			if (dataLen)
			{
				sConnectedSocket *cl = priv_2280_findClientByUID(uid);
				if (NULL != cl)
				{
					rhea::socket::write (cl->sok, data, dataLen);
					logger->log ("rcv [%d] bytes from RS232, sending to socket [%d]\n", dataLen, cl->uid);
				}
				else
				{
					DBGBREAK;
				}
			}

			//rimuovo il msg dal buffer di input
			b.removeFirstNBytes(10+dataLen);
			return true;
		}
		break;
	}
}


//*******************************************************
Core::sConnectedSocket* Core::priv_2280_findClientByUID (u32 uid)
{
	for (u32 i = 0; i < clientList.getNElem(); i++)
	{
		if (clientList(i).uid == uid)
			return &clientList[i];
	}
	return NULL;
}

//*******************************************************
void Core::priv_2280_accept()
{
	sConnectedSocket cl;
	if (!rhea::socket::accept (sok2280, &cl.sok))
	{
		logger->log("ERR => accept failed on 2280\n");
		return;
	}

	//ok, ho accettato una socket
	//Gli assegno un id univoco
	cl.uid = ++sok2280NextID;

	//comunico via seriale che ho accettato una nuova socket
	const u32 nBytesToSend = esapi::buildMsg_R0x01_newSocket (cl.uid, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
	priv_rs232_sendBuffer (rs232BufferOUT, nBytesToSend);

	//aggiungo la socket alla lista dei client connessi
	clientList.append (cl);
	waitableGrp.addSocket (cl.sok, cl.uid);

	logger->log ("socket [%d] connected\n", cl.uid);
}

/*********************************************************
 * Ho ricevuto dei dati lungo la socket, li spedisco via rs232 alla GPU
 */
void Core::priv_2280_onIncomingData (OSSocket &sok, u32 uid)
{
	sConnectedSocket *cl = priv_2280_findClientByUID(uid);
	if (NULL == cl)
	{
		DBGBREAK;
		return;
	}
	assert (rhea::socket::compare(cl->sok, sok));


	i32 nBytesLetti = rhea::socket::read (cl->sok, sok2280Buffer, SOK_BUFFER_SIZE, 100);
	if (nBytesLetti == 0)
	{
		//connessione chiusa
		priv_2280_onClientDisconnected(sok, uid);
		return;
	}
	if (nBytesLetti < 0)
	{
		//la chiamata sarebbe stata bloccante, non dovrebbe succedere
		DBGBREAK;
		return;
	}

	//spedisco lungo la seriale
	const u32 nBytesToSend = esapi::buildMsg_R0x03_socketDataToGPU (cl->uid, sok2280Buffer, nBytesLetti, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
	priv_rs232_sendBuffer (rs232BufferOUT, nBytesToSend);
	logger->log ("rcv [%d] bytes from socket [%d], sending to GPU\n", nBytesLetti, cl->uid);

}

//*********************************************************
void Core::priv_2280_onClientDisconnected (OSSocket &sok, u32 uid)
{
	for (u32 i = 0; i < clientList.getNElem(); i++)
	{
		if (clientList(i).uid == uid)
		{
			assert (rhea::socket::compare(clientList(i).sok, sok));

			waitableGrp.removeSocket (clientList[i].sok);
			rhea::socket::close(clientList[i].sok);
			clientList.removeAndSwapWithLast(i);

			//comunico la disconnessione via seriale
			const u32 nBytesToSend = esapi::buildMsg_R0x02_closeSocket (uid, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
			priv_rs232_sendBuffer (rs232BufferOUT, nBytesToSend);
			logger->log ("socket [%d] disconnected\n", uid);
			return;
		}
	}

	//non ho trovato il client nella lista...
	DBGBREAK;

}