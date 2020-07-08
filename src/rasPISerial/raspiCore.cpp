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
    rhea::socket::init (&sok2281);
    fileUpload.f = NULL;
	fileUpload.lastTimeRcvMSec = 0;

    memset (&hotspot, 0, sizeof(hotspot));
    hotspot.bIsOn=1;
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
    //se non esiste gi�, creo la cartella temp
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
    sok2281BufferIN = (u8*)RHEAALLOC(localAllocator, SOK2281_BUFFERIN_SIZE);
    sok2281BufferOUT = (u8*)RHEAALLOC(localAllocator, SOK2281_BUFFEROUT_SIZE);

	//recupero il mio IP di rete wifi
    //NB: il codice sottostante � perferttamente funzionane, recupera l'IP interrogando l'interfaccia di rete
    //Dato pero' che il processo parte prima che l'interfaccia di rete wifi sia effettivamente online, l'IP che riesce a recuperare � solo quello
    //di localhost, wlan0 non � ancora pronta.
    //Per risolvere il problema, c'� uno script python che crea un file con dentro l'IP corretto. Leggo l'IP da quel file
#if 0
    memset (hotspot.wifiIP, 0, sizeof(hotspot.wifiIP));
	{
		u32 n = 0;
		sNetworkAdapterInfo *ipList = rhea::netaddr::getListOfAllNerworkAdpaterIPAndNetmask (rhea::getScrapAllocator(), &n);
		if (n)
		{
			for (u32 i = 0; i < n; i++)
			{
				if (strcasecmp (ipList[i].name, "wlan0") == 0)
				{
                    rhea::netaddr::ipstrTo4bytes (ipList[i].ip, &hotspot.wifiIP[0], &hotspot.wifiIP[1], &hotspot.wifiIP[2], &hotspot.wifiIP[3]);
					break;
				}
			}
			
            if (hotspot.wifiIP[0] == 0)
			{
				//questo � per la versione win, che non ha wlan0
                rhea::netaddr::ipstrTo4bytes (ipList[0].ip, &hotspot.wifiIP[0], &hotspot.wifiIP[1], &hotspot.wifiIP[2], &hotspot.wifiIP[3]);
			}

			RHEAFREE(rhea::getScrapAllocator(), ipList);
            logger->log ("WIFI IP: %d.%d.%d.%d\n", hotspot.wifiIP[0], hotspot.wifiIP[1], hotspot.wifiIP[2], hotspot.wifiIP[3]);
		}
	}
#endif

    memset (hotspot.wifiIP, 0, sizeof(hotspot.wifiIP));
    {
        u8 s[128];
        sprintf_s ((char*)s, sizeof(s), "%s/ip.txt", rhea::getPhysicalPathToAppFolder());
        FILE *f = rhea::fs::fileOpenForReadBinary(s);
        if (NULL == f)
        {
            logger->log ("ERR: unable to open file [%s]\n", s);
        }
        else
        {
            memset (s, 0, sizeof(s));
            fread (s, 32, 1, f);
            fclose(f);

            rhea::netaddr::ipstrTo4bytes ((const char*)s, &hotspot.wifiIP[0], &hotspot.wifiIP[1], &hotspot.wifiIP[2], &hotspot.wifiIP[3]);
        }
    }


    //recupero SSID dell'hotspot
    //Uno script python parte allo startup del rasPI e crea un file di testo di nome "hotspotname.txt" che contiene il nome dell'hotspot
    memset (hotspot.ssid, 0, sizeof(hotspot.ssid));
    {
        u8 s[128];
        sprintf_s ((char*)s, sizeof(s), "%s/hotspotname.txt", rhea::getPhysicalPathToAppFolder());
        FILE *f = rhea::fs::fileOpenForReadBinary(s);
        if (NULL == f)
        {
            logger->log ("ERR: unable to open file [%s]\n", s);
            sprintf_s ((char*)hotspot.ssid, sizeof(hotspot.ssid), "unknown");
        }
        else
        {
            fread (hotspot.ssid, sizeof(hotspot.ssid), 1, f);
            fclose(f);
        }
    }
    logger->log ("Hotspot name:%s\n", hotspot.ssid);

	return true;
}

//*****************************************************************
void Core::priv_close ()
{
	rhea::rs232::close (com);
	rhea::socket::close (sok2280);
    rhea::socket::close (sok2281);

	if (localAllocator)
	{
		RHEAFREE(localAllocator, rs232BufferOUT);
		rs232BufferIN.free (localAllocator);
        RHEAFREE(localAllocator, sok2280Buffer);
        RHEAFREE(localAllocator, sok2281BufferIN);
        RHEAFREE(localAllocator, sok2281BufferOUT);
        clientList.unsetup();
		RHEADELETE(rhea::getSysHeapAllocator(), localAllocator);
		localAllocator = NULL;
	}
}

//*********************************************************
u32 Core::priv_esapi_buildMsg (u8 c1, u8 c2, const u8* optionalData, u32 numOfBytesInOptionalData, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	const u32 totalSizeOfMsg = 4 + numOfBytesInOptionalData;
	if (sizeOfOutBuffer < totalSizeOfMsg)
	{
		DBGBREAK;
		return 0;
	}

    u32 ct = 0;
    out_buffer[ct++] = '#';
    out_buffer[ct++] = c1;
	out_buffer[ct++] = c2;
    if (NULL != optionalData && numOfBytesInOptionalData)
    {
        memcpy (&out_buffer[ct], optionalData, numOfBytesInOptionalData);
        ct += numOfBytesInOptionalData;
    }

    out_buffer[ct] = rhea::utils::simpleChecksum8_calc (out_buffer, ct);
    ct++;

	return ct;
}

//****************************************************************************
bool Core::priv_esapi_isValidChecksum (u8 ck, const u8 *buffer, u32 numBytesToUse)
{
	return (ck == rhea::utils::simpleChecksum8_calc(buffer, numBytesToUse));
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

    //flush del buffer seriale nel caso ci sia qualche schifezza
    rhea::rs232::readBuffer(com, rs232BufferIN.buffer, rs232BufferIN.SIZE);
    rs232BufferIN.numBytesInBuffer=0;

    logger->log ("requesting API version...\n");
    u64 timeToSendMsgMSec = 0;
	while (1)
	{
		const u64 timeNowMSec = rhea::getTimeNowMSec();
		if (timeNowMSec >= timeToSendMsgMSec)
		{
            timeToSendMsgMSec = timeNowMSec + 150;
            logger->log (".");

			const u32 nBytesToSend = priv_esapi_buildMsg ('A', '1', NULL, 0, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
			priv_rs232_sendBuffer (rs232BufferOUT, nBytesToSend);
			if (priv_identify_waitAnswer('A', '1', 7, rs232BufferOUT, 1000))
			{
				//ho ricevuto risposta valida a comando A 1
				reportedESAPIVerMajor = rs232BufferOUT[3];
				reportedESAPIVerMinor = rs232BufferOUT[4];
				reportedGPUType = (esapi::eGPUType)rs232BufferOUT[5];
                logger->log ("\nAPI ver %d.%d, gpuType[%d]\n", reportedESAPIVerMajor, reportedESAPIVerMinor, reportedGPUType);
				break;
			}
		}
        else
            rhea::thread::sleepMSec(50);
	}


	//se arrivo qui, vuol dire che la GPU ha risposto al comando # A 1
	//ora devo comunicare la mia identit� e attendere risposta
	while (1)
	{
		const u8 data[4] = { (u8)esapi::eExternalModuleType_rasPI_wifi_REST, VER_MAJOR, VER_MINOR, 0 };
		const u32 nBytesToSend = priv_esapi_buildMsg ('R', '1', data, 3, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
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
 * In questa fase, il modulo � slave, ovvero attende comandi da GPU via seriale.
 * Questa fase termina alla ricezione del comando # R 0x01 [ck] che manda il modulo
 * nella modali� operativa vera e propria
 */
void Core::priv_boot_run()
{
#ifdef LINUX
    u64 timeToSyncMSec = 0;
#endif

    bQuit = false;
    while (bQuit == false)
    {
		priv_boot_rs232_handleCommunication(rs232BufferIN);
        rhea::thread::sleepMSec(100);

#ifdef LINUX
        //flusho i dati su SD nel tentativo di preservare l'SD da spegnimenti improvvisi
        if (rhea::getTimeNowMSec() >= timeToSyncMSec)
        {
            timeToSyncMSec = rhea::getTimeNowMSec() + 10000;
            sync();
        }
#endif
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

#ifdef _DEBUG
		if (0 == nBytesAvailInBuffer)
			DBGBREAK;
#endif

	    if (nBytesAvailInBuffer > 0)
	    {
		    const u32 nRead = rhea::rs232::readBuffer(com, &b.buffer[b.numBytesInBuffer], nBytesAvailInBuffer);
		    b.numBytesInBuffer += (u16)nRead;
	    }
    
		if (0 == b.numBytesInBuffer)
			return;

        //provo ad estrarre un msg 'raw' dal mio buffer.
        //Cerco il carattere di inizio messaggio (#) ed eventualmente butto via tutto quello che c'� prima
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
            if (!priv_esapi_isValidChecksum (b.buffer[3], b.buffer, 3))
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
            if (!priv_esapi_isValidChecksum (b.buffer[3], b.buffer, 3))
            {
                b.removeFirstNBytes(1);
                break;
            }

            //rimuovo il msg dal buffer
            b.removeFirstNBytes(4);

            //rispondo # R [0x02] [ip1] [ip2] [ip3] [ip4] [lenSSID] [ssidString...] [ck]
            {
                const u8 lenSSID = (u8)rhea::string::utf8::lengthInBytes(hotspot.ssid);
                u32 ct = 0;
                rs232BufferOUT[ct++] = '#';
                rs232BufferOUT[ct++] = 'R';
                rs232BufferOUT[ct++] = 0x02;
                rs232BufferOUT[ct++] = hotspot.wifiIP[0];
                rs232BufferOUT[ct++] = hotspot.wifiIP[1];
                rs232BufferOUT[ct++] = hotspot.wifiIP[2];
                rs232BufferOUT[ct++] = hotspot.wifiIP[3];
                rs232BufferOUT[ct++] = lenSSID;
                memcpy (&rs232BufferOUT[ct], hotspot.ssid, lenSSID);
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
                if (!priv_esapi_isValidChecksum (b.buffer[totalMsgLen - 1], b.buffer, totalMsgLen - 1))
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
                if (!priv_esapi_isValidChecksum (b.buffer[expectedMsgLen-1], b.buffer, expectedMsgLen-1))
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

				if (!priv_esapi_isValidChecksum (b.buffer[totalLenOfMsg - 1], b.buffer, totalLenOfMsg - 1))
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
                {
                    priv_boot_finalizeGUITSInstall(dst);
					result = 0x01;
                }
				priv_boot_buildMsgBufferAndSend (rs232BufferOUT, SIZE_OF_RS232BUFFEROUT, 0x05, &result, 1);
				logger->log ("unzip resul [%d]\n", result);

#ifdef LINUX
                //dovrebbe flushare tutti i dati sulla SD
                sync();
#endif
			}
			break;
        }

    } //while(1)

}

//*********************************************************
void Core::priv_boot_finalizeGUITSInstall (const u8* const pathToGUIFolder)
{
    logger->log("priv_finalizeGUITSInstall [%s]\n", pathToGUIFolder);
    logger->incIndent();

    //per prima cosa devo cambiare l'IP usato dalla websocket per collegarsi al server.
    //Al posto di 127.0.0.1 ci devo mettere l'IP di questa macchina
    //L'ip si trova all'interno del file js/rhea_final.min.js
    u8 s[512];
    u32 filesize = 0;
    sprintf_s ((char*)s, sizeof(s), "%s/js/rhea_final.min.js", pathToGUIFolder);
    u8 *pSRC = rhea::fs::fileCopyInMemory (s, rhea::getScrapAllocator(), &filesize);
    if (NULL == pSRC)
    {
        logger->log ("ERR: unable to load file [%s] in memory\n", s);
    }
    else
    {
        char myIP[16];
        sprintf_s (myIP, sizeof(myIP), "%d.%d.%d.%d", hotspot.wifiIP[0], hotspot.wifiIP[1], hotspot.wifiIP[2], hotspot.wifiIP[3]);
        const u32 myIPLen = strlen(myIP);

        const u8 toFind[] = { "127.0.0.1" };
        const u32 toFindLen = rhea::string::utf8::lengthInBytes(toFind);

        if (filesize >= toFindLen)
        {
            for (u32 i = 0; i < filesize-toFindLen; i++)
            {
                if (memcmp (&pSRC[i], toFind, toFindLen) == 0)
                {
                    logger->log ("found [%s], replacing with [%s]\n", toFind, myIP);
                    u8 *buffer = (u8*)RHEAALLOC(rhea::getScrapAllocator(), filesize + 32);
                    memcpy (buffer, pSRC, i);
                    memcpy (&buffer[i], myIP, myIPLen);
                    memcpy (&buffer[i+myIPLen], &pSRC[i+toFindLen], filesize - i - toFindLen);


                    sprintf_s ((char*)s, sizeof(s), "%s/js/rhea_final.min.js", pathToGUIFolder);
                    logger->log ("opening file [%s] for write\n", s);
                    FILE *f = rhea::fs::fileOpenForWriteBinary(s);
                    if (NULL == f)
                    {
                        logger->log ("ERR: unable to write to file  [%s]\n", s);
                    }
                    else
                    {
                        rhea::fs::fileWrite (f, buffer, filesize - toFindLen + myIPLen);
                        fclose(f);
                        logger->log ("done\n");
                    }

                    RHEAFREE(rhea::getScrapAllocator(), buffer);
                    i = u32MAX-1;
                }
            }
        }

        RHEAFREE(rhea::getScrapAllocator(), pSRC);
    }

    //Poi devo copiare la cartella coi font
    u8 s2[512];
    sprintf_s ((char*)s, sizeof(s), "%s/gui_parts/fonts", rhea::getPhysicalPathToAppFolder());
    sprintf_s ((char*)s2, sizeof(s2), "%s/fonts", pathToGUIFolder);
    logger->log ("copying folder [%s] into [%s]\n", s, s2);
    rhea::fs::folderCopy (s, s2, NULL);

    logger->log ("end\n");
    logger->decIndent();
}




//*******************************************************
void Core::priv_openSocket2280()
{
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
        return;
    }

    logger->log("OK\n");

    //aggiungo la socket al gruppo di oggetti in osservazione
    waitableGrp.addSocket(sok2280, WAITGRP_SOCKET2280);
}

//*******************************************************
void Core::priv_openSocket2281()
{
    logger->log ("opening socket on 2281...");
    eSocketError err = rhea::socket::openAsTCPServer(&sok2281, 2281);
    if (err != eSocketError_none)
    {
        logger->log ("ERR code[%d]\n", err);
        logger->log("\n");
    }
    else
        logger->log("OK\n");

    rhea::socket::setReadTimeoutMSec(sok2281, 0);
    rhea::socket::setWriteTimeoutMSec(sok2281, 10000);

    logger->log("listen... ");
    if (!rhea::socket::listen(sok2281))
    {
        logger->log("FAIL\n", err);
        logger->decIndent();
        rhea::socket::close(sok2281);
        return;
    }

    logger->log("OK\n");

    //aggiungo la socket al gruppo di oggetti in osservazione
    waitableGrp.addSocket(sok2281, WAITGRP_SOCKET2281);
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
        priv_openSocket2280();
        priv_openSocket2281();
	logger->decIndent();

#ifdef LINUX
    u64 timeToSyncMSec = 0;
    waitableGrp.addSerialPort (com, WAITGRP_RS232);
#endif

	bQuit = false;
	while (bQuit == false)
	{
#ifdef LINUX
        const u8 nEvents = waitableGrp.wait(5000);
#else
        const u8 nEvents = waitableGrp.wait(100);
#endif
		for (u8 i = 0; i < nEvents; i++)
		{
			if (waitableGrp.getEventOrigin(i) == OSWaitableGrp::evt_origin_socket)
			{
                if (waitableGrp.getEventUserParamAsU32(i) == WAITGRP_SOCKET2280)
				{
                    priv_2280_accept();
				}
                else if (waitableGrp.getEventUserParamAsU32(i) == WAITGRP_SOCKET2281)
                {
                    OSSocket sok;
                    if (!rhea::socket::accept (sok2281, &sok))
                        logger->log("ERR => accept failed on 2281\n");
                    else
                    {
                        logger->log ("accepted on 2281\n");
                        waitableGrp.addSocket (sok, WAITGRP_SOK_FROM_REST_API);
                    }
                }
				else
				{
					//altimenti la socket che si � svegliata deve essere una dei miei client gi� connessi
					const u32 clientUID = waitableGrp.getEventUserParamAsU32(i);
                    if (WAITGRP_SOK_FROM_REST_API == clientUID)
                        priv_2281_handle_restAPI(waitableGrp.getEventSrcAsOSSocket (i));
                    else
                    {
                        OSSocket sok = waitableGrp.getEventSrcAsOSSocket (i);
                        priv_2280_onIncomingData (sok, clientUID);
                    }
				}
			}
#ifdef LINUX
            else if (waitableGrp.getEventOrigin(i) == OSWaitableGrp::evt_origin_serialPort)
            {
                if (waitableGrp.getEventUserParamAsU32(i) == WAITGRP_RS232)
                {
                    priv_rs232_handleIncomingData(rs232BufferIN);
                }
            }
#endif
        }

        //se hotspot � stato spento, verifico se � ora di riaccenderlo (vedi comando REST/SUSP-WIFI)
        if (hotspot.timeToTurnOnMSec)
        {
            if (rhea::getTimeNowMSec() >= hotspot.timeToTurnOnMSec)
            {
                logger->log ("2281: turn on hotspot\n");
                hotspot.timeToTurnOnMSec = 0;
                hotspot.turnON();
            }
        }

#ifdef LINUX
        //flusho i dati su SD nel tentativo di preservare l'SD da spegnimenti improvvisi
        if (rhea::getTimeNowMSec() >= timeToSyncMSec)
        {
            timeToSyncMSec = rhea::getTimeNowMSec() + 10000;
            sync();
        }
#endif

#ifndef LINUX
        priv_rs232_handleIncomingData(rs232BufferIN);
#endif
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
		//Cerco il carattere di inizio buffer ed eventualmente butto via tutto quello che c'� prima
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
		//# A 1 [api_ver_major] [api_ver_minor] [GPUmodel] [ck]
		if (b.numBytesInBuffer < 7)
			return false;
		if (!priv_esapi_isValidChecksum(b.buffer[6], b.buffer, 6))
		{
			b.removeFirstNBytes(2);
			return true;
		}

		reportedESAPIVerMajor = b.buffer[3];
		reportedESAPIVerMinor = b.buffer[4];
		reportedGPUType = (esapi::eGPUType)b.buffer[5];
		logger->log ("reported ESAPI version [%d].[%d], GPUType[%d]\n", reportedESAPIVerMajor, reportedESAPIVerMinor, reportedGPUType);

		//rimuovo il msg dal buffer di input
		b.removeFirstNBytes(7);
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
		//GPU mi comunica che la socket xxx � stata chiusa
		//# R [0x02] [socketUID_MSB3] [socketUID_MSB2] [socketUID_MSB1] [socketUID_LSB] [ck]
		if (b.numBytesInBuffer < 8)
			return false;
		if (rhea::utils::simpleChecksum8_calc(b.buffer, 7) != b.buffer[7])
		{
			DBGBREAK;
			b.removeFirstNBytes(2);
			return true;
		}

		//parse del messaggio
		{
			const u32 socketUID = rhea::utils::bufferReadU32(&b.buffer[3]);

			//rimuovo msg dal buffer
			b.removeFirstNBytes(8);

			//elimino il client
			sConnectedSocket *cl = priv_2280_findClientByUID (socketUID);
			if (cl)
				priv_2280_onClientDisconnected (cl->sok, socketUID);
		}
		return true;

	case 0x04:
		//GPU mi sta comunicando dei dati che io devo mandare lungo la socket indicata
		//rcv:   # R [0x04] [client_uid_MSB3] [client_uid_MSB2] [client_uid_MSB1] [client_uid_LSB] [lenMSB] [lenLSB] [data�] [ck]
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
	//# R [0x01] [socketUID_MSB3] [socketUID_MSB2] [socketUID_MSB1] [socketUID_LSB] [ck]
	u8 data[4];
	rhea::utils::bufferWriteU32 (data, cl.uid);
	const u32 nBytesToSend = priv_esapi_buildMsg ('R', 0x01, data, 4, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
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
	//# R [0x03] [socketUID_MSB3] [socketUID_MSB2] [socketUID_MSB1] [socketUID_LSB] [lenMSB] [lenLSB] [data�] [ck]
    u32 ct = 0;
    rs232BufferOUT[ct++] = '#';
    rs232BufferOUT[ct++] = 'R';
	rs232BufferOUT[ct++] = 0x03;
	
	rhea::utils::bufferWriteU32 (&rs232BufferOUT[ct], cl->uid);
	ct += 4;

    rhea::utils::bufferWriteU16 (&rs232BufferOUT[ct], (u16)nBytesLetti);
	ct += 2;

    memcpy (&rs232BufferOUT[ct], sok2280Buffer, (u16)nBytesLetti);
    ct += (u16)nBytesLetti;

    rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc (rs232BufferOUT, ct);
    ct++;

	priv_rs232_sendBuffer (rs232BufferOUT, ct);
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
			//# R [0x02] [socketUID_MSB3] [socketUID_MSB2] [socketUID_MSB1] [socketUID_LSB] [ck]
			u8 data[4];
			rhea::utils::bufferWriteU32 (data, uid);
			const u32 nBytesToSend= priv_esapi_buildMsg ('R', 0x02, data, 4, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
			priv_rs232_sendBuffer (rs232BufferOUT, nBytesToSend);
			logger->log ("socket [%d] disconnected\n", uid);
			return;
		}
	}

	//non ho trovato il client nella lista...
	DBGBREAK;

}

//*********************************************************
void Core::priv_2281_handle_restAPI (OSSocket &sok)
{
    i32 nBytesLetti = rhea::socket::read (sok, sok2281BufferIN, SOK2281_BUFFERIN_SIZE, 100);
    if (nBytesLetti == 0)
    {
        //connessione chiusa
        rhea::socket::close(sok);
        logger->log ("2281: closed\n");
        return;
    }
    if (nBytesLetti < 0)
    {
        //la chiamata sarebbe stata bloccante, non dovrebbe succedere
        DBGBREAK;
        return;
    }
    sok2281BufferIN[nBytesLetti] = 0x00;

    if (nBytesLetti < 3)
    {
        logger->log ("2281: invalid input [%s]\n", sok2281BufferIN);
        return;
    }

    const rhea::UTF8Char    cSep(124);          // pipe
    rhea::string::utf8::Iter iter;
    rhea::UTF8Char c;
    u8  command[64];
    command[0] = 0x00;
    iter.setup (sok2281BufferIN);
    while ( !(c = iter.getCurChar()).isEOF() )
    {
        if (c == cSep)
        {
            //comando con parametri
            iter.copyStrFromXToCurrentPosition (0, command, sizeof(command), false);
            iter.advanceOneChar();
            priv_2281_handle_singleCommand (command, &iter);
            return;
        }

        iter.advanceOneChar();
    }

    //comando senza parametri
    priv_2281_handle_singleCommand (sok2281BufferIN, NULL);
    sok2281BufferIN[0] = 0;
}

//*********************************************************
bool Core::priv_2281_utils_match (const u8 *command, u32 commandLen, const char *match) const
{
    const u32 n = strlen(match);
    if (commandLen != n)
        return false;
    return rhea::string::utf8::areEqual (command, (const u8*)match, true);
}

//*********************************************************
void Core::priv_2281_handle_singleCommand (const u8 *command, rhea::string::utf8::Iter *params)
{
    const u32 commandLen = rhea::string::utf8::lengthInBytes(command);

    if (NULL == params)
    {
        //comando senza parametri
    }
    else
    {
        //comando con parametri
        if (priv_2281_utils_match(command, commandLen, "SUSP-WIFI"))
        {
            //SUSP-WIFI|timeSec
            i32 timeSec=3;
            rhea::string::utf8::extractInteger (*params, &timeSec);
            logger->log ("2281: [%s] [%d]\n", command, timeSec);

            logger->log ("2281: turn off hotspot\n");
            hotspot.timeToTurnOnMSec = rhea::getTimeNowMSec() + (timeSec*1000);
            hotspot.turnOFF();
            return;
        }
    }

    logger->log ("2281: ERR command [%s] not recognized\n", command);
}
