#include "rasPIMITMCore.h"
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
		b.removeFirstNBytes(msgLen);

		//gestisco il messaggio
		switch ((char)msg[1])
		{
		case 'W':
			//messaggio speciale, da decodificare e passare al mio attuale subscriber, oppure da gestire direttamente in MITM.
			//Se i primi 2 byte del payload sono 0xFF 0xFF, allora è un msg per MITM, atrimenti è per il subscriber
			//# W [len] .... [ck]
			if (msg[3] == 0xff && msg[4] == 0xff)
			{
				priv_handleInternalWMessages(msg);
			}
			else if (bDoIHaveASubscriber)
			{
				u16         what = 0;
				u32         paramU32 = 0;
				u32         bufferSize = 0;
				const u8	*bufferPt = NULL;
				rhea::thread::deserializMsg (&msg[3], msgLen-4, &what, &paramU32, &bufferSize, &bufferPt);
				rhea::thread::pushMsg (subscriberSocketListener.hFromCpuToOtherW, what, paramU32, bufferPt, bufferSize);
			}
			break;

		default:
			//un generico messaggio:
			//Lo giro pari pari alla CPU e attendo risposta
			{
				priv_handleMsg_send (comCPU, msg, msgLen);
				u16 answerLen = (u16)sizeof(msg);
				const bool answerReceived = priv_handleMsg_rcv (comCPU, msg, &answerLen, TIMEOUT_CPU_ANSWER_MSec);

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
						priv_handleMsg_send (comGPU, &bufferSpontaneousMsgForGPU.buffer[ct], n);
						ct += n;
						bufferSpontaneousMsgForGPU.numBytesInBuffer -= n;
					}
					bufferSpontaneousMsgForGPU.numBytesInBuffer = 0;
				}

				//mando a GPU la risposta di CPU
				if (answerReceived)
					priv_handleMsg_send (comGPU, msg, answerLen);
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
	const u8 expectedMsgLen = p[2];

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
 * priv_handleMsg_send
 *
 *	true se il buffer è stato spedito interamente
 *	false in caso contrario. In ogni caso, entro 2000Msec la fn ritorna false
 */
bool Core::priv_handleMsg_send (OSSerialPort &comPort, const u8 *buffer, u16 nBytesToSend)
{
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

//***************************************************
bool Core::priv_handleMsg_rcv (OSSerialPort &comPort, u8 *out_answer, u16 *in_out_sizeOfAnswer, u64 timeoutRCVMsec)
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
				logger->log("Core::priv_handleMsg_rcv() => ERR answer len is [%d], out_buffer len is only [%d] bytes\n", msgLen, sizeOfBuffer);
				return false;
			}
			if (msgLen < 4)
			{
				logger->log("Core::priv_handleMsg_rcv() => ERR invalid msg len [%d]\n", msgLen);
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

				logger->log("Core::priv_handleMsg_rcv() => ERR, invalid checksum\n", msgLen, sizeOfBuffer);
				return false;
			}
		}
	}

	logger->log("Core::priv_handleMsg_rcv() => ERR, timeout rcv\n\n");
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
				u8 tempBuffer[1024];
				const u32 msgLen = rhea::thread::serializeMsg (msg, tempBuffer, sizeof(tempBuffer));
				if (msgLen)
				{
					assert (msgLen <= (0xff - 4));
					const u32 startOfMsg = bufferSpontaneousMsgForGPU.numBytesInBuffer;
					bufferSpontaneousMsgForGPU.appendU8 ((u8)'#');
					bufferSpontaneousMsgForGPU.appendU8 ((u8)'W');
					bufferSpontaneousMsgForGPU.appendU8 ((u8)(msgLen + 4));
					bufferSpontaneousMsgForGPU.append (tempBuffer, msgLen);

					const u8 ck = rhea::utils::simpleChecksum8_calc (&bufferSpontaneousMsgForGPU.buffer[startOfMsg], msgLen + 3);
					bufferSpontaneousMsgForGPU.appendU8 (ck);

					logger->log ("adding [bufferSpontaneousMsgForGPU]\n");
				}
				else
				{
					DBGBREAK;
				}
			}
			else
				logger->log ("rasPI::MITM::priv_handleIncomingMsgFromThreadQ() => unhandled msg from thread msgQ [what=%d]\n", msg.what);
			break;
		}
		
		
		rhea::thread::deleteMsg(msg);
    }
}

//*********************************************************
bool Core::priv_buildAndSendMsgWToGPU (u8 command, const u8 *optionalData, u16 sizeOfOptionaData)
{
	u8 msg[256];

	//calcolo della dimensione totale
	//# W [len] [0xff] [0xff] [command] .... [ck]
	if (sizeof(msg) < 7 + sizeOfOptionaData)
	{
		DBGBREAK;
		return false;
	}

	u8 ct = 0;
	msg[ct++] = '#';
	msg[ct++] = 'W';
	msg[ct++] = 0; //length
	msg[ct++] = 0xff;
	msg[ct++] = 0xff;
	msg[ct++] = command;

	if (optionalData && sizeOfOptionaData)
	{
		memcpy(&msg[ct], optionalData, sizeOfOptionaData);
		ct += sizeOfOptionaData;
	}

	msg[2] = (ct+1);	//length
	msg[ct] = rhea::utils::simpleChecksum8_calc(msg, ct);
	ct++;
	
	priv_handleMsg_send (comGPU, msg, ct);
	return true;
}

//*********************************************************
void Core::priv_handleInternalWMessages(const u8 *msg)
{
	//Ho ricevuto un msg 'W' da GPU.
	//Questo msg è speficico per MITM, non va inoltrato al mio subscriber
	//# W [len] [0xff] [0xff] [command] .... [ck]
	switch (msg[5])
	{
	default:
		logger->log ("ERR MITM::priv_handleInternalWMessages() => invalid msg [%d]\n", msg[5]);
		break;

	case CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_ARE_YOU_THERE:
		//E' una sorta di ping che GPU invia per sapere se il modulo MITM esiste
		//Rispondo con lo stesso msg e aggiungo 3 byte per usi futuri
		{
			const u8 optionalData[3] = { 0,0,0 };
			priv_buildAndSendMsgWToGPU (msg[5], optionalData, 3);
		}
		break;


	case CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_START_SOCKETBRIDGE:
		//GPU mi comunica che posso uscire dalla fase di "boot" e lanciare socketbridge in modo da
		//permettere agli utenti web di accedere all'interfaccia
		{
			rhea::HThread hSocketBridgeThread;
			socketbridge::startServer(logger, subscriberSocketListener.hFromOtherToCpuW, false, &hSocketBridgeThread);

			//rispondo con lo stesso msg indicando 0x01 per dire che tutto ok
			const u8 optionalData = 0x01;
			priv_buildAndSendMsgWToGPU (msg[5], &optionalData, 1);
		}
		break;
	}
}