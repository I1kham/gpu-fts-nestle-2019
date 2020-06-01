#include "rasPIMITMCore.h"
#include "../CPUBridge/CPUBridge.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../SocketBridge/SocketBridge.h"
#include "../rheaCommonLib/compress/rheaCompress.h"

using namespace rasPI;
using namespace rasPI::MITM;


//*********************************************************
Core::Core()
{
	localAllocator = NULL;
	logger = &nullLogger;
    comGPUName[0] = comCPUName[0] = 0x00;
    rhea::rs232::setInvalid (comGPU);
    rhea::rs232::setInvalid (comCPU);
	bDoIHaveASubscriber = false;
}

//*********************************************************
void Core::useLogger (rhea::ISimpleLogger *loggerIN)
{
    if (NULL==loggerIN)
        logger = &nullLogger;
    else
        logger = loggerIN;
}

//*****************************************************************
void Core::priv_close ()
{
	rhea::rs232::close (comGPU);
	rhea::rs232::close (comCPU);
    if (localAllocator)
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(subscriberSocketListener.hFromOtherToCpuR, &h);
        waitableGrp.removeEvent (h);

        rhea::thread::deleteMsgQ (subscriberSocketListener.hFromOtherToCpuR, subscriberSocketListener.hFromOtherToCpuW);
		rhea::thread::deleteMsgQ (subscriberSocketListener.hFromCpuToOtherR, subscriberSocketListener.hFromCpuToOtherW);
		
		priv_freeBuffer(bufferGPU);
		priv_freeBuffer(bufferSpontaneousMsgForGPU);

        RHEADELETE(rhea::getSysHeapAllocator(), localAllocator);
        localAllocator = NULL;
    }
}

//*********************************************************
bool Core::open (const char *serialPortGPU, const char *serialPortCPU)
{
	const bool SERIAL_IS_BLOCKING = false;

	logger->log ("rasPI::MITM::open\n");
    logger->incIndent();

	logger->log ("comGPU=%s   ", serialPortGPU);
    sprintf_s (comGPUName, sizeof(comGPUName), "%s", serialPortGPU);
    if (!rhea::rs232::open(&comGPU, comGPUName, eRS232BaudRate_115200, false, false, eRS232DataBits_8, eRS232Parity_No, eRS232StopBits_One, eRS232FlowControl_No, SERIAL_IS_BLOCKING))
    {
        logger->log ("FAILED. unable to open port [%s]\n", comGPUName);
        logger->decIndent();
        return false;
    }
	logger->log ("OK\n");

	logger->log ("comCPU=%s   ", serialPortCPU);
    sprintf_s (comCPUName, sizeof(comCPUName), "%s", serialPortCPU);
    if (!rhea::rs232::open(&comCPU, comCPUName, eRS232BaudRate_115200, false, false, eRS232DataBits_8, eRS232Parity_No, eRS232StopBits_One, eRS232FlowControl_No, SERIAL_IS_BLOCKING))
    {
        rhea::rs232::close (comGPU);
        logger->log ("FAILED. unable to open port [%s]\n", comCPUName);
        logger->decIndent();
        return false;
    }
	logger->log ("OK\n");

    localAllocator = RHEANEW(rhea::getSysHeapAllocator(), rhea::AllocatorSimpleWithMemTrack) ("mitm");

    //crea le msgq per questo thread
	// [hFromOtherToCpuR, hFromOtherToCpuW] � la msgQ sulla quale "subscriber" pu� postare messaggi per comunicare a MITM
	// [hFromCpuToOtherR, hFromCpuToOtherW] � la msgQ sulla quale MITM posta messaggi per comunicare con subscriber
    rhea::thread::createMsgQ (&subscriberSocketListener.hFromOtherToCpuR, &subscriberSocketListener.hFromOtherToCpuW);
	rhea::thread::createMsgQ (&subscriberSocketListener.hFromCpuToOtherR, &subscriberSocketListener.hFromCpuToOtherW);

    //aggiungo l'OSEvent della msgQ alla waitList in modo da essere notificato quando subscriber invia un msg
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(subscriberSocketListener.hFromOtherToCpuR, &h);
        waitableGrp.addEvent (h, WAITLIST_EVENT_FROM_SUBSCRIBER_MSGQ);
    }

	//buffer vari
	priv_allocBuffer (&bufferGPU, SIZE_OF_BUFFER_GPU);
	priv_allocBuffer (&bufferSpontaneousMsgForGPU, 4096);
	

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
				//questo � per la versione win, che non ha wlan0
				rhea::netaddr::ipstrTo4bytes (ipList[0].ip, &wifiIP[0], &wifiIP[1], &wifiIP[2], &wifiIP[3]);
			}

			RHEAFREE(rhea::getScrapAllocator(), ipList);
			logger->log ("WIFI IP: %d.%d.%d.%d\n", wifiIP[0], wifiIP[1], wifiIP[2], wifiIP[3]);
		}
	}


	logger->log ("OK\n");
	logger->decIndent();
    return true;
}

//*********************************************************
void Core::priv_allocBuffer(sBuffer *out, u16 max_size)
{
	out->SIZE = max_size;
	out->numBytesInBuffer = 0;
	out->buffer = (u8*)RHEAALLOC(localAllocator, max_size);
}

//*********************************************************
void Core::priv_freeBuffer (sBuffer &b)
{
	b.numBytesInBuffer = 0;
	b.SIZE = 0;
	if (NULL != b.buffer)
		RHEAFREE(localAllocator, b.buffer);
	b.buffer = NULL;
}

//*********************************************************
void Core::run()
{
	static const u32 MAX_WAIT_TIME_MSec = 100;

	u32 throttle = 0;
	u32 waitTimeMSec = MAX_WAIT_TIME_MSec;
    bQuit = false;
    while (bQuit == false)
    {
		//qui sarebbe bello rimanere in wait per sempre fino a che un evento non scatta oppure la seriale ha dei 
		//dati in input.
		//TODO: trovare un modo multipiattaforma per rimanere in wait sulla seriale (in linux � facile, � windows che rogna un po')
        const u8 nEvents = waitableGrp.wait(waitTimeMSec);

		for (u8 i = 0; i < nEvents; i++)
		{
			switch (waitableGrp.getEventOrigin(i))
			{
			case OSWaitableGrp::evt_origin_osevent:
				{
					if (WAITLIST_EVENT_FROM_SUBSCRIBER_MSGQ == waitableGrp.getEventUserParamAsU32(i))
					{
						priv_handleIncomingMsgFromSubscriber();
					}
					else
					{
						DBGBREAK;
						logger->log ("rasPI::MITM::run() => unhandled OSEVENT from waitGrp [%d]\n", waitableGrp.getEventUserParamAsU32(i));
					}
				}
				break;

			default:
				logger->log ("rasPI::MITM::run() => unhandled event from waitGrp [%d]\n", waitableGrp.getEventOrigin(i));
				break;
			}
		}

		//gestione comunicazione seriale
		u32 newWaitTime = 0;
        if (priv_handleSerialCommunication (comGPU, bufferGPU))
		{
			//ho ricevuto qualche msg da parte della GPU
			//Provo ad aggiustare il tempo di wait, l'idea � che se ricevo tanti msg di seguito, non sto
			//ad aspettare 100 ms tra un check e l'altro
			if (throttle >= 10)
				throttle-=10;
			else
				throttle = 0;
		}
		else
		{
			throttle++;
			newWaitTime = throttle / 5;
			if (newWaitTime > MAX_WAIT_TIME_MSec)
			{
				newWaitTime = MAX_WAIT_TIME_MSec;
				throttle--;
			}
		}
		
		if (newWaitTime != waitTimeMSec)
		{
			waitTimeMSec = newWaitTime;
			//logger->log ("WAIT TIME: %d\n", waitTimeMSec);
		}
    }

    priv_close();
}

//*********************************************************
void Core::priv_utils_printMsg (const char *prefix, const OSSerialPort &comPort, const u8 *buffer, u32 nBytes)
{
	return;
#ifdef _DEBUG
	logger->log (prefix);
	if (memcmp(&comPort, &comGPU, sizeof(OSSerialPort)) == 0)
		logger->log ("GPU: "); 
	else
        logger->log ("CPU: ");

	for (u32 i = 0; i < nBytes; i++)
	{
		const u8 c = buffer[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '#')
			logger->log ("%c   ", c);
		else
			logger->log ("%03d ", c);
	}
	logger->log ("\n");
#else
#endif
}

//*********************************************************
bool Core::priv_handleSerialCommunication (OSSerialPort &comPort, sBuffer &b)
{
	bool ret = false;
	while (1)
	{
		//leggo tutto quello che posso dalla seriale e bufferizzo in [b]
		const u16 nBytesAvailInBuffer = b.SIZE - b.numBytesInBuffer;
		if (nBytesAvailInBuffer > 0)
		{
			const u32 nRead = rhea::rs232::readBuffer(comPort, &b.buffer[b.numBytesInBuffer], nBytesAvailInBuffer);
			b.numBytesInBuffer += (u16)nRead;
            //if (nRead)	logger->log ("read %d bytes from serial port: buffer size %d\n", nRead, b.numBytesInBuffer);
		}

		//provo ad estrarre un msg GPU/CPU dal mio buffer
        u8 msg[1024];
        const u16 bytesConsumed = priv_extractMessage (b, msg, sizeof(msg));
        if (0 == bytesConsumed)
			return ret;
		ret = true;

        //shifto il buffer per eliminare i bytes consumati
        b.removeFirstNBytes(bytesConsumed);

        //se ho un valido messaggiom , lo gestisco
        if (msg[0]=='#')
        {
            switch ((char)msg[1])
            {
            case 'W':
                //messaggio speciale diretto a me, da non passare alla CPU
                //# W lenLSB lenMSB ... ck
                priv_utils_printMsg("rcv from ", comPort, msg, (u16)msg[2] + 256*(u16)msg[3]);
                priv_handleInternalWMessages(msg);
                break;

            default:
                //un generico messaggio: # command len ... ck
                //Lo giro pari pari alla CPU e attendo risposta
                {
                    priv_serial_send (comCPU, msg, msg[2]);

                    u16 answerLen = (u16)sizeof(msg);
                    u32 timeoutMSec = TIMEOUT_CPU_ANSWER_MSec;
                    switch (msg[1])
                    {
                    case 'C':
                        //il comando C deve rispondere in fretta perch� � usato massicciamente all'avvio della GPU, non posso stare
                        //5 secondi in attesa di risposta se dall'altro lato non ci fosse la CPU
                        timeoutMSec = 200;
                        break;

                    case 'c':
                        timeoutMSec = 1000;
                        break;
                    }

                    const bool answerReceived = priv_serial_waitMsg (comCPU, msg, &answerLen, timeoutMSec);

                    //Questo � il momento di mandare a GPU eventuali messaggio che il mio subscriber mi aveva chiesto di inviare
                    //Prima mando questi messaggio e poi, per ultimo, mando il msg di risposta della CPU
                    if (bufferSpontaneousMsgForGPU.numBytesInBuffer)
                    {
                        logger->log ("sending [bufferSpontaneousMsgForGPU]\n");
                        u32 ct = 0;
                        while (bufferSpontaneousMsgForGPU.numBytesInBuffer)
                        {
                            const u32 n = priv_isAValidMessage (&bufferSpontaneousMsgForGPU.buffer[ct], bufferSpontaneousMsgForGPU.numBytesInBuffer);
                            assert (n <= bufferSpontaneousMsgForGPU.numBytesInBuffer);
                            priv_serial_send (comGPU, &bufferSpontaneousMsgForGPU.buffer[ct], n);
                            ct += n;
                            bufferSpontaneousMsgForGPU.numBytesInBuffer -= n;
                        }
                        bufferSpontaneousMsgForGPU.numBytesInBuffer = 0;
                    }

                    //mando a GPU la risposta di CPU
                    if (answerReceived)
                        priv_serial_send (comGPU, msg, answerLen);
                }
                break;
            }
        }
	}

	return ret;
}

//*********************************************************
u32 Core::priv_extractMessage (sBuffer &b, u8 *out, u32 sizeOfOut)
{
    out[0] = 0;
	if (b.numBytesInBuffer < 4)
		return 0;

	for (u32 i = 0; i < b.numBytesInBuffer; i++)
	{
		if (b.buffer[i] == (u8)'#')
		{
			const u32 nBytesUsed = priv_isAValidMessage(&b.buffer[i], b.numBytesInBuffer - i);
			if (nBytesUsed)
			{
				assert (sizeOfOut >= nBytesUsed);
				memcpy (out, &b.buffer[i], nBytesUsed);
                return i + nBytesUsed;
			}
            return i;
		}
	}

    return b.numBytesInBuffer;
}

//*********************************************************
u32 Core::priv_isAValidMessage (const u8 *p, u32 nBytesToCheck) const
{
	if (nBytesToCheck < 4)
		return 0;
	if (p[0] != (u8)'#')
		return 0;

	const char command = (const char)p[1];

	u16 expectedMsgLen;
	if (command == 'W')
	{
		//� un messaggio speciale del protocollo rasPI, supporta una lunghezza max di 0xffff
		expectedMsgLen = rhea::utils::bufferReadU16_LSB_MSB(&p[2]);
	}
	else
	{
		//� un classico msg CPU-GPU
		expectedMsgLen = p[2];
	}

	if (expectedMsgLen < 4)
		return 0;
	if (nBytesToCheck < expectedMsgLen)
		return 0;

	const u8 receivedCK = p[expectedMsgLen - 1];
	const u8 calculateCK = rhea::utils::simpleChecksum8_calc(p, expectedMsgLen - 1);
	if (receivedCK == calculateCK)
		return expectedMsgLen;
	return 0;
}


/***************************************************
 * priv_serial_send
 *
 *	true se il buffer � stato spedito interamente
 *	false in caso contrario. In ogni caso, entro 2000Msec la fn ritorna false
 */
bool Core::priv_serial_send (OSSerialPort &comPort, const u8 *buffer, u16 nBytesToSend)
{
	if (0 == nBytesToSend)
		return true;

    //if (buffer[1] == 'W')
		priv_utils_printMsg("snd to ", comPort, buffer, nBytesToSend);

	const u64 timeToExitMSec = rhea::getTimeNowMSec() + 2000;
	u16 nBytesSent = 0;
	
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		u32 nToWrite = (nBytesToSend - nBytesSent);
		nBytesSent += rhea::rs232::writeBuffer(comPort, &buffer[nBytesSent], nToWrite);
		if (nBytesSent >= nBytesToSend)
			return true;
	}

    logger->log ("Core::priv_serial_send() ERROR snd\n");
	return false;
}

/***************************************************
 * priv_serial_waitMsg
 *
 *	si aspetta di riceve una valido messaggio nel classico formato CPU-GPU
 *	true se lo riceve entro il timeout, false altrimenti
 */
bool Core::priv_serial_waitMsg (OSSerialPort &comPort, u8 *out_answer, u16 *in_out_sizeOfAnswer, u64 timeoutRCVMsec)
{
	const u16 sizeOfBuffer = *in_out_sizeOfAnswer;
	*in_out_sizeOfAnswer = 0;

	const u64 timeToExitMSec = rhea::getTimeNowMSec() + timeoutRCVMsec;
	u16 nBytesRcv = 0;
	u8	commandChar = 0x00;
	u8	msgLen = 0x00;
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		//aspetto di ricevere un #
		if (nBytesRcv == 0)
		{
			u8 b = 0x00;
			if (0 == rhea::rs232::readBuffer(comPort, &b, 1))
				continue;

			if (b != (u8)'#')
				continue;
			nBytesRcv++;
		}

		//aspetto di riceve un carattere qualunque subito dopo il #
		if (nBytesRcv == 1)
		{
			if (0 == rhea::rs232::readBuffer(comPort, &commandChar, 1))
				continue;
			nBytesRcv++;
		}

		//aspetto di riceve la lunghezza totale del messaggio
		if (nBytesRcv == 2)
		{
			if (0 == rhea::rs232::readBuffer(comPort, &msgLen, 1))
				continue;
			nBytesRcv++;

			if (sizeOfBuffer < msgLen)
			{
				logger->log("Core::priv_serial_waitMsg() => ERR answer len is [%d], out_buffer len is only [%d] bytes\n", msgLen, sizeOfBuffer);
				return false;
			}
			if (msgLen < 4)
			{
				logger->log("Core::priv_serial_waitMsg() => ERR invalid msg len [%d]\n", msgLen);
				return false;
			}

			out_answer[0] = '#';
			out_answer[1] = commandChar;
			out_answer[2] = msgLen;
			*in_out_sizeOfAnswer = msgLen;
		}

		//cerco di recuperare tutto il resto del msg
		if (nBytesRcv >= 3)
		{
			const u16 nMissing = msgLen - nBytesRcv;
			const u16 nLetti = (u16)rhea::rs232::readBuffer(comPort, &out_answer[nBytesRcv], nMissing);

			nBytesRcv += nLetti;
			if (nBytesRcv >= msgLen)
			{
				//eval checksum
				if (out_answer[msgLen - 1] == rhea::utils::simpleChecksum8_calc(out_answer, msgLen - 1))
					return true;

				logger->log("Core::priv_serial_waitMsg() => ERR, invalid checksum\n", msgLen, sizeOfBuffer);
				return false;
			}
		}
	}

    logger->log("Core::priv_serial_waitMsg() => ERR, timeout rcv {timeout was %d ms]\n\n", (u32)timeoutRCVMsec);
	return false;
}

//***************************************************
bool Core::priv_serial_waitChar(OSSerialPort &comPort, u64 timeoutMSec, u8 *out_char)
{
	const u64 timeToExitMSec = rhea::getTimeNowMSec() + timeoutMSec;
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		if (1 == rhea::rs232::readBuffer(comPort, out_char, 1))
			return true;
	}

	*out_char = 0x00;
	return false;
}

//*********************************************************
void Core::priv_handleIncomingMsgFromSubscriber()
{
	rhea::thread::sMsg msg;
    while (rhea::thread::popMsg (subscriberSocketListener.hFromOtherToCpuR, &msg))
    {
		switch (msg.what)
		{
		case CPUBRIDGE_SERVICECH_SUBSCRIPTION_REQUEST:
			//rispondo al thread richiedente
			{
				bDoIHaveASubscriber = true;
				HThreadMsgW hToThreadW;
				hToThreadW.initFromU32 (msg.paramU32);
				rhea::thread::pushMsg (hToThreadW, CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER, (u32)0x01, &subscriberSocketListener, sizeof(subscriberSocketListener));
			}
			break;

		default:
			if (msg.what >= CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION && msg.what <= 0x8ff)
			{
				//� un messaggio di tipo "CPUBRIDGE_SUBSCRIBER_ASK_.." inviato dal mio subscriber e che devo inviare alla GPU
				//lungo la seriale. Creo quindi un msg "W" ad hoc
				const u32 ct = bufferSpontaneousMsgForGPU.numBytesInBuffer;
				const u16 n = cpubridge::buildMsg_rasPI_MITM_serializedSMsg (msg, &bufferSpontaneousMsgForGPU.buffer[ct], bufferSpontaneousMsgForGPU.SIZE - ct);
				bufferSpontaneousMsgForGPU.numBytesInBuffer += n;
				logger->log ("adding [bufferSpontaneousMsgForGPU]\n");
			}
			else
				logger->log ("rasPI::MITM::priv_handleIncomingMsgFromThreadQ() => unhandled msg from thread msgQ [what=%d]\n", msg.what);
			break;
		}
		
		
		rhea::thread::deleteMsg(msg);
    }
}

//*********************************************************
void Core::priv_handleInternalWMessages(const u8 *msg)
{
	//Ho ricevuto un msg 'W' da GPU.
	//Questo msg � speficico per MITM, non va inoltrato al mio subscriber
	//# W [len1] [len2] [subcommand] .... [ck]
	const eRasPISubcommand subcommand = (eRasPISubcommand)msg[4];
	const u8 *payload = &msg[5];
	const u16 totalMsgLen = rhea::utils::bufferReadU16_LSB_MSB(&msg[2]);
	switch (subcommand)
	{
	default:
		DBGBREAK;
		logger->log ("ERR MITM::priv_handleInternalWMessages() => invalid msg [%d]\n", subcommand);
		break;

	case eRasPISubcommand_SERIALIZED_sMSG:
		if (bDoIHaveASubscriber)
		{
			u16         what = 0;
			u32         paramU32 = 0;
			u32         bufferSize = 0;
			const u8	*bufferPt = NULL;
			rhea::thread::deserializMsg (payload, &what, &paramU32, &bufferSize, &bufferPt);
			rhea::thread::pushMsg (subscriberSocketListener.hFromCpuToOtherW, what, paramU32, bufferPt, bufferSize);
		}
		break;


	case eRasPISubcommand_ARE_YOU_THERE:
		//E' una sorta di ping che GPU invia per sapere se il modulo MITM esiste
		//Rispondo con lo stesso msg includendo versione e 3 byte per usi futuri
		{
			u8 bufferW[32];
			const u8 optionalData[4] = { RASPI_MODULE_VERSION, 0,0,0 };
			const u16 nToSend = cpubridge::buildMsg_rasPI_MITM (subcommand, optionalData, 4, bufferW, sizeof(bufferW));
			priv_serial_send (comGPU, bufferW, nToSend);
		}
		break;


	case eRasPISubcommand_START_SOCKETBRIDGE:
		//GPU mi comunica che posso uscire dalla fase di "boot" e lanciare socketbridge in modo da
		//permettere agli utenti web di accedere all'interfaccia
		{
			rhea::HThread hSocketBridgeThread;
			socketbridge::startServer(logger, subscriberSocketListener.hFromOtherToCpuW, false, false, &hSocketBridgeThread);

			//rispondo con lo stesso msg indicando 0x01 per dire che tutto ok
			u8 bufferW[16];
			const u8 optionalData = 0x01;
			const u16 nToSend = cpubridge::buildMsg_rasPI_MITM (subcommand, &optionalData, 1, bufferW, sizeof(bufferW));
			priv_serial_send (comGPU, bufferW, nToSend);
		}
		break;

	case eRasPISubcommand_SEND_AND_DO_NOT_WAIT:
		//la gpu vuole che io mandi un tot di dati direttamente a CPU e che poi non aspetti alcuna risposta
		{
            logger->log ("SEND_AND_DO_NOT_WAIT\n");
			const u16 payloadLen = totalMsgLen - 6;
			priv_serial_send (comCPU, payload, payloadLen);
		}
		break;

	case eRasPISubcommand_WAIT_SPECIFIC_CHAR:
		//GPU vuole che io stia in attesa di un singolo char in arrivo dalla CPU
		//Il char in questione deve essere precisamente quello indicato dal payload, eventuali altri char in arrivo da CPU sono 
		//scartati. Se il char in questione arriva entro il timeout, rispondo alla GPU, altrimenti niente
		{
			const u32 timeoutMSec = rhea::utils::bufferReadU32(payload);
			const u8 specificChar = payload[4];

            logger->log ("WAIT_SPECIFIC_CHAR [c=%d, timeout=%dms\n", specificChar, timeoutMSec);

            const u64 timeToExitMSec = rhea::getTimeNowMSec() + timeoutMSec;
			while (rhea::getTimeNowMSec() < timeToExitMSec)
			{
				u8 c;
				if (priv_serial_waitChar(comCPU, timeoutMSec, &c))
				{
					if (c == specificChar)
					{
						rhea::rs232::writeBuffer (comGPU, &specificChar, 1);
                        logger->log ("specific char was sent to GPU\n");
						break;
					}
				}
			}
		}
		break;

	case eRasPISubcommand_GET_WIFI_IP:
		{
			//rispondo con lo stesso msg indicando 4 byte dell'IP
			u8 bufferW[16];
			const u16 nToSend = cpubridge::buildMsg_rasPI_MITM (subcommand, wifiIP, 4, bufferW, sizeof(bufferW));
			priv_serial_send (comGPU, bufferW, nToSend);
		}
		break;

	case eRasPISubcommand_UPLOAD_BEGIN:
		//GPU vuole inziare un file-upload
		//Entro in un loop fino alla fine del trasferimento
		priv_handleFileUpload(msg);
		break;

	case eRasPISubcommand_UNZIP_TS_GUI:
		//GPU vuole che io unzippi un file che ho nella mia cartella temp e lo metta
		//nella cartella dedicata alle GUI TS
		{
			u8 src[512];
			u8 dst[512];
#ifdef LINUX
#ifdef PLATFORM_UBUNTU_DESKTOP
			rhea::fs::extractFileNameWithoutExt (payload, src, sizeof(src));
			sprintf_s ((char*)dst, sizeof(dst), "%s/temp/%s/%s", rhea::getPhysicalPathToAppFolder(), src, payload);
#else
			sprintf_s ((char*)dst, sizeof(dst), "/var/www/html/GUITS");
#endif
#else
			rhea::fs::extractFileNameWithoutExt (payload, src, sizeof(src));
			sprintf_s ((char*)dst, sizeof(dst), "%s/temp/%s/%s", rhea::getPhysicalPathToAppFolder(), src, payload);
#endif
			u8 bufferW[16];
			sprintf_s ((char*)src, sizeof(src), "%s/temp/%s", rhea::getPhysicalPathToAppFolder(), payload);
			if (rhea::CompressUtility::decompresAll (src, dst))
				src[0] = 0x01;
			else
				src[0] = 0x00;

			//rispondo
			src[1] = 0x00;
			const u16 n = cpubridge::buildMsg_rasPI_MITM_unzipTouchscreenGUI (src, bufferW, sizeof(bufferW));
			priv_serial_send (comGPU, bufferW, n);
		}
		break;

	} //switch
}

//*********************************************************
void Core::priv_handleFileUpload(const u8 *msg)
{
	const u32 TIMEOU_MSEC = 3000;
	const eRasPISubcommand subcommand = (eRasPISubcommand)msg[4];
	const u8 *payload = &msg[5];
	const u16 totalMsgLen = rhea::utils::bufferReadU16_LSB_MSB(&msg[2]);

	assert (subcommand == eRasPISubcommand_UPLOAD_BEGIN);
	u32 fileLenInBytes = rhea::utils::bufferReadU32(payload);
	const u16 packetSize = rhea::utils::bufferReadU16(&payload[4]);
	const u8* fileName = (const u8*)&payload[6];

	logger->log ("priv_handleFileUpload() => starting upload of [%s] [size:%d] [pcktsize:%d]\n", fileName, fileLenInBytes, packetSize);
	u32 BUFFER_SIZE = 2048;
	if (packetSize*2 > BUFFER_SIZE)
		BUFFER_SIZE = packetSize*2;
	u8 *buffer = (u8*)RHEAALLOC (rhea::getScrapAllocator(), BUFFER_SIZE);
	
	
	sprintf_s ((char*)buffer , BUFFER_SIZE, "%s/temp/%s", rhea::getPhysicalPathToAppFolder(), fileName);
	FILE *fDST = rhea::fs::fileOpenForWriteBinary(buffer);
	if (NULL == fDST)
	{
		//rispondo KO
		u8 data[4];
		data[0] = 0;
		const u16 nToSend = cpubridge::buildMsg_rasPI_MITM (eRasPISubcommand_UPLOAD_BEGIN, data, 1, buffer, BUFFER_SIZE);
		priv_serial_send (comGPU, buffer, nToSend);
		RHEAFREE(rhea::getScrapAllocator(), buffer);
		logger->log ("priv_handleFileUpload() => ERR: unable to open file for write [%s]\n", buffer);
		return;
	}

	//rispondo OK
	{
		u8 data[4];
		data[0] = 1;
		const u16 nToSend = cpubridge::buildMsg_rasPI_MITM (eRasPISubcommand_UPLOAD_BEGIN, data, 1, buffer, BUFFER_SIZE);
		priv_serial_send (comGPU, buffer, nToSend);
	}

	//rimango in attesa dei pacchetti
	u32 numPacketOfSizeEqualToPacketSize = fileLenInBytes / packetSize;
	u32 nBytesInBuffer = 0;
	u64 timeoutMSec = rhea::getTimeNowMSec() + TIMEOU_MSEC;
	while (numPacketOfSizeEqualToPacketSize && rhea::getTimeNowMSec() < timeoutMSec)
	{
		const u32 nRead = rhea::rs232::readBuffer(comGPU, &buffer[nBytesInBuffer], BUFFER_SIZE - nBytesInBuffer);
		nBytesInBuffer += nRead;
		while (nBytesInBuffer >= packetSize)
		{
			logger->log ("priv_handleFileUpload() => packet rcv [%d]\n", numPacketOfSizeEqualToPacketSize);
			fileLenInBytes -= packetSize;
			numPacketOfSizeEqualToPacketSize--;
			rhea::fs::fileWrite (fDST, buffer, packetSize);

			//invio conferma ricezione
			const u32 ck = rhea::utils::simpleChecksum16_calc (buffer, packetSize);
			buffer[0] = (u8)((ck & 0xff00) >> 8);
			buffer[1] = (u8)(ck & 0x00ff);
			logger->log ("priv_handleFileUpload() => snd ck=%d\n", ck);
			priv_serial_send (comGPU, buffer, 2);


			//shifto il buffer per vedere se ho altri packet bufferizzati
			nBytesInBuffer -= packetSize;
			if (nBytesInBuffer)
				memcpy (buffer, &buffer[packetSize], nBytesInBuffer);

			//aggiorno timeout
			timeoutMSec = rhea::getTimeNowMSec() + TIMEOU_MSEC;
		}
	}

	const u32 sizeOfLastPacket = fileLenInBytes;
	if (sizeOfLastPacket && fileLenInBytes < packetSize)
	{
		logger->log ("priv_handleFileUpload() => waiting last packet\n");
		timeoutMSec = rhea::getTimeNowMSec() + TIMEOU_MSEC;
		while (rhea::getTimeNowMSec() < timeoutMSec)
		{
			const u32 nRead = rhea::rs232::readBuffer(comGPU, &buffer[nBytesInBuffer], BUFFER_SIZE - nBytesInBuffer);
			nBytesInBuffer += nRead;
			if (nBytesInBuffer >= sizeOfLastPacket)
			{
				logger->log ("priv_handleFileUpload() => last packet rcv\n");
				fileLenInBytes -= sizeOfLastPacket;
				rhea::fs::fileWrite (fDST, buffer, sizeOfLastPacket);

				//invio conferma ricezione
				const u32 ck = rhea::utils::simpleChecksum16_calc (buffer, sizeOfLastPacket);
				buffer[0] = (u8)((ck & 0xff00) >> 8);
				buffer[1] = (u8)(ck & 0x00ff);
				logger->log ("priv_handleFileUpload() => snd ck=%d\n", ck);
				priv_serial_send (comGPU, buffer, 2);
				break;
			}
		}
	}

	fclose(fDST);
	RHEAFREE(rhea::getScrapAllocator(), buffer);

	if (0 == fileLenInBytes)
		logger->log ("priv_handleFileUpload() => finished OK\n");
	else
		logger->log ("priv_handleFileUpload() => finished KO\n");

}