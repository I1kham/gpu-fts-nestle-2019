#include "rasPIMITMCore.h"
#include "../CPUBridge/CPUBridge.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../SocketBridge/SocketBridge.h"

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
	// [hFromOtherToCpuR, hFromOtherToCpuW] è la msgQ sulla quale "subscriber" può postare messaggi per comunicare a MITM
	// [hFromCpuToOtherR, hFromCpuToOtherW] è la msgQ sulla quale MITM posta messaggi per comunicare con subscriber
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
    bQuit = false;
    while (bQuit == false)
    {
		//qui sarebbe bello rimanere in wait per sempre fino a che un evento non scatta oppure la seriale ha dei 
		//dati in input.
		//TODO: trovare un modo multipiattaforma per rimanere in wait sulla seriale (in linux è facile, è windows che rogna un po')
        const u8 nEvents = waitableGrp.wait(100);

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
		priv_handleSerialCommunication (comGPU, bufferGPU);
    }

    priv_close();
}

//*********************************************************
void Core::priv_utils_printMsg (const u8 *buffer, u32 nBytes)
{
	for (u32 i = 0; i < nBytes; i++)
	{
		const u8 c = buffer[i];
		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '#')
			logger->log ("%c ", c);
		else
			logger->log ("%03d ", c);
	}
	logger->log ("\n");
}

//*********************************************************
void Core::priv_handleSerialCommunication (OSSerialPort &comPort, sBuffer &b)
{
	while (1)
	{
		//leggo tutto quello che posso dalla seriale e bufferizzo in [b]
		const u16 nBytesAvailInBuffer = b.SIZE - b.numBytesInBuffer;
		if (nBytesAvailInBuffer > 0)
		{
			const u32 nRead = rhea::rs232::readBuffer(comPort, &b.buffer[b.numBytesInBuffer], nBytesAvailInBuffer);
			b.numBytesInBuffer += (u16)nRead;
		}

		//provo ad estrarre un msg GPU/CPU dal mio buffer
		u8 msg[256];
		const u16 msgLen = priv_extractMessage (b, msg, sizeof(msg));
		if (0 == msgLen)
			return;

		//ho un valido messaggio in msg, shifto il buffer per eliminarlo dalla coda
		if (memcmp(&comPort, &comGPU, sizeof(OSSerialPort)) == 0)
			logger->log ("rcv from GPU: "); 
		else
			logger->log ("rcv from CPU: "); 
		priv_utils_printMsg(msg, msgLen);
		
		b.removeFirstNBytes(msgLen);

		//gestisco il messaggio
		switch ((char)msg[1])
		{
		case 'W':
			//messaggio speciale diretto a me, da non passare alla CPU
			priv_handleInternalWMessages(msg);
			break;

		default:
			//un generico messaggio:
			//Lo giro pari pari alla CPU e attendo risposta
			{
				priv_serial_send (comCPU, msg, msgLen);
				u16 answerLen = (u16)sizeof(msg);
				const bool answerReceived = priv_serial_waitMsg (comCPU, msg, &answerLen, TIMEOUT_CPU_ANSWER_MSec);

				//Questo è il momento di mandare a GPU eventuali messaggio che il mio subscriber mi aveva chiesto di inviare
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

//*********************************************************
u32 Core::priv_extractMessage (sBuffer &b, u8 *out, u32 sizeOfOut)
{
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
		}
	}

	return 0;
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
		//è un messaggio speciale del protocollo rasPI, supporta una lunghezza max di 0xffff
		expectedMsgLen = rhea::utils::bufferReadU16_LSB_MSB(&p[2]);
	}
	else
	{
		//è un classico msg CPU-GPU
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
 *	true se il buffer è stato spedito interamente
 *	false in caso contrario. In ogni caso, entro 2000Msec la fn ritorna false
 */
bool Core::priv_serial_send (OSSerialPort &comPort, const u8 *buffer, u16 nBytesToSend)
{
	if (0 == nBytesToSend)
		return true;

	if (memcmp(&comPort, &comGPU, sizeof(OSSerialPort)) == 0)
		logger->log ("snd to GPU: "); 
	else
		logger->log ("snd to CPU: "); 
	priv_utils_printMsg(buffer, nBytesToSend);

	const u64 timeToExitMSec = rhea::getTimeNowMSec() + 2000;
	u16 nBytesSent = 0;
	
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		u32 nToWrite = (nBytesToSend - nBytesSent);
		nBytesSent += rhea::rs232::writeBuffer(comPort, &buffer[nBytesSent], nToWrite);
		if (nBytesSent >= nBytesToSend)
			return true;
	}
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

	logger->log("Core::priv_serial_waitMsg() => ERR, timeout rcv\n\n");
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
				//è un messaggio di tipo "CPUBRIDGE_SUBSCRIBER_ASK_.." inviato dal mio subscriber e che devo inviare alla GPU
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
	//Questo msg è speficico per MITM, non va inoltrato al mio subscriber
	//# W [len1] [len2] [subcommand] .... [ck]
	const u8 subcommand = msg[4];
	const u8 *payload = &msg[5];
	const u16 totalMsgLen = rhea::utils::bufferReadU16_LSB_MSB(&msg[2]);
	switch (subcommand)
	{
	default:
		DBGBREAK;
		logger->log ("ERR MITM::priv_handleInternalWMessages() => invalid msg [%d]\n", subcommand);
		break;

	case RASPI_MITM_COMMANDW_SERIALIZED_sMSG:
		if (bDoIHaveASubscriber)
		{
			u16         what = 0;
			u32         paramU32 = 0;
			u32         bufferSize = 0;
			const u8	*bufferPt = NULL;
			rhea::thread::deserializMsg (&msg[3], &what, &paramU32, &bufferSize, &bufferPt);
			rhea::thread::pushMsg (subscriberSocketListener.hFromCpuToOtherW, what, paramU32, bufferPt, bufferSize);
		}
		break;


	case RASPI_MITM_COMMANDW_ARE_YOU_THERE:
		//E' una sorta di ping che GPU invia per sapere se il modulo MITM esiste
		//Rispondo con lo stesso msg includendo versione e 3 byte per usi futuri
		{
			u8 bufferW[32];
			const u8 optionalData[4] = { RASPI_MODULE_VERSION, 0,0,0 };
			const u16 nToSend = cpubridge::buildMsg_rasPI_MITM (subcommand, optionalData, 4, bufferW, sizeof(bufferW));
			priv_serial_send (comGPU, bufferW, nToSend);
		}
		break;


	case RASPI_MITM_COMMANDW_START_SOCKETBRIDGE:
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

	case RASPI_MITM_COMMANDW_SEND_AND_DO_NOT_WAIT:
		//la gpu vuole che io mandi un tot di dati direttamente a CPU e che poi non aspetti alcuna risposta
		{
			const u16 payloadLen = totalMsgLen - 6;
			priv_serial_send (comCPU, payload, payloadLen);
		}
		break;

	case RASPI_MITM_COMMANDW_MITM_WAIT_SPECIFIC_CHAR:
		//GPU vuole che io stia in attesa di un singolo char in arrivo dalla CPU
		//Il char in questione deve essere precisamente quello indicato dal payload, eventuali altri char in arrivo da CPU sono 
		//scartati. Se il char in questione arriva entro il timeout, rispondo alla GPU, altrimenti niente
		{
			const u32 timeoutMSec = rhea::utils::bufferReadU32(payload);
			const u8 specificChar = payload[4];

			const u64 timeToExitMSec = rhea::getTimeNowMSec() + timeoutMSec;
			while (rhea::getTimeNowMSec() < timeToExitMSec)
			{
				u8 c;
				if (priv_serial_waitChar(comCPU, timeoutMSec, &c))
				{
					if (c == specificChar)
					{
						rhea::rs232::writeBuffer (comGPU, &specificChar, 1);
						break;
					}
				}
			}
		}
		break;
	}
}