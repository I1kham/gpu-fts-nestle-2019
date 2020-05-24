#include "rasPIMITMCore.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaUtils.h"

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
        rhea::thread::getMsgQEvent(msqQR, &h);
        waitableGrp.removeEvent (h);

        rhea::thread::deleteMsgQ (msqQR, msqQW);
		
		priv_freeBuffer(bufferGPU);
		priv_freeBuffer(bufferSpontaneousMsgForGPU);

        RHEADELETE(rhea::getSysHeapAllocator(), localAllocator);
        localAllocator = NULL;
    }
}

//*********************************************************
bool Core::open (const char *serialPortGPU, const char *serialPortCPU)
{
	const bool SERIAL_IS_BLOCING = false;

	logger->log ("rasPI::MITM::open\n");
    logger->incIndent();

	logger->log ("comGPU=%s   ", serialPortGPU);
    sprintf_s (comGPUName, sizeof(comGPUName), "%s", serialPortGPU);
    if (!rhea::rs232::open(&comGPU, comGPUName, eRS232BaudRate_115200, false, false, eRS232DataBits_8, eRS232Parity_No, eRS232StopBits_One, eRS232FlowControl_No, SERIAL_IS_BLOCING))
    {
        logger->log ("FAILED. unable to open port [%s]\n", comGPUName);
        logger->decIndent();
        return false;
    }
	logger->log ("OK\n");

	logger->log ("comCPU=%s   ", serialPortCPU);
    sprintf_s (comCPUName, sizeof(comCPUName), "%s", serialPortCPU);
    if (!rhea::rs232::open(&comCPU, comCPUName, eRS232BaudRate_115200, false, false, eRS232DataBits_8, eRS232Parity_No, eRS232StopBits_One, eRS232FlowControl_No, SERIAL_IS_BLOCING))
    {
        rhea::rs232::close (comGPU);
        logger->log ("FAILED. unable to open port [%s]\n", comCPUName);
        logger->decIndent();
        return false;
    }
	logger->log ("OK\n");

    localAllocator = RHEANEW(rhea::getSysHeapAllocator(), rhea::AllocatorSimpleWithMemTrack) ("mitm");

    //crea la msgq per questo thread
    rhea::thread::createMsgQ (&msqQR, &msqQW);

    //aggiungo l'OSEvent della msgQ alla waitList
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(msqQR, &h);
        waitableGrp.addEvent (h, WAITLIST_EVENT_FROM_THREAD_MSGQ);
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
        const u8 nEvents = waitableGrp.wait(100);

		for (u8 i = 0; i < nEvents; i++)
		{
			switch (waitableGrp.getEventOrigin(i))
			{
			case OSWaitableGrp::evt_origin_osevent:
				{
					if (WAITLIST_EVENT_FROM_THREAD_MSGQ == waitableGrp.getEventUserParamAsU32(i))
					{
						//ho un messaggio nella mia msgQ
						priv_handleIncomingMsgFromThreadQ();
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

		priv_handleIncomingGPUMsg (comGPU, bufferGPU);
    }

    priv_close();
}

//*********************************************************
void Core::priv_handleIncomingGPUMsg (OSSerialPort &comPort, sBuffer &b)
{
	while (1)
	{
		const u16 nBytesAvailInBuffer = b.SIZE - b.numBytesInBuffer;
		if (nBytesAvailInBuffer > 0)
		{
			const u32 nRead = rhea::rs232::readBuffer(comPort, &b.buffer[b.numBytesInBuffer], nBytesAvailInBuffer);
			b.numBytesInBuffer += (u16)nRead;
		}

		u8 msg[256];
		const u16 msgLen = priv_extractMessage (b, msg, sizeof(msg));
		if (0 == msgLen)
			return;

		//ho un valido messaggio in msg, shifto il buffer per eliminare il msg dalla coda
		b.removeFirstNBytes(msgLen);

		//gestisco il messaggio
		switch ((char)msg[1])
		{
		case 'W':
			//messaggio speciale, indirizzato a rasPI, non lo devo passare alla CPU
			break;

		default:
			//un generico messaggio:
			//Lo giro pari pari alla CPU e attendo risposta
			{
				priv_handleMsg_send (comCPU, msg, msgLen);
				u16 answerLen = (u16)sizeof(msg);
				if (priv_handleMsg_rcv (comCPU, msg, &answerLen, TIMEOUT_CPU_ANSWER_MSec))
				{
					//rigiro la risposta a GPU
					//Questo è il momento di mandare eventuali messaggi specifici per la GPU che avevo in coda
					if (bufferSpontaneousMsgForGPU.numBytesInBuffer)
					{
						u32 ct = 0;
						while (bufferSpontaneousMsgForGPU.numBytesInBuffer)
						{
							const u32 n = priv_isAValidMessage (&bufferSpontaneousMsgForGPU.buffer[ct], bufferSpontaneousMsgForGPU.numBytesInBuffer);
							assert (n <= bufferSpontaneousMsgForGPU.numBytesInBuffer);
							ct += n;
							bufferSpontaneousMsgForGPU.numBytesInBuffer -= n;
							priv_handleMsg_send (comGPU, &bufferSpontaneousMsgForGPU.buffer[ct], n);
						}
						bufferSpontaneousMsgForGPU.numBytesInBuffer = 0;
					}

					//mando a GPU la risposta di CPU
					priv_handleMsg_send (comGPU, msg, answerLen);
				}
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
void Core::Core::priv_handleIncomingMsgFromThreadQ()
{
	rhea::thread::sMsg msg;
    while (rhea::thread::popMsg (msqQR, &msg))
    {
		switch ((eRASPI_MITM_MSGQ)msg.what)
		{
		case eRASPI_MITM_MSGQ_QUIT:
			bQuit = true;
			break;

		case eRASPI_MITM_MSGQ_SEND_BUFFER_TO_GPU:
			{
				//creo un msg "W" ad hoc
				assert (msg.bufferSize <= (0xff-4));
				const u32 startOfMsg = bufferSpontaneousMsgForGPU.numBytesInBuffer;
				bufferSpontaneousMsgForGPU.appendU8 ((u8)'#');
				bufferSpontaneousMsgForGPU.appendU8 ((u8)'W');
				bufferSpontaneousMsgForGPU.appendU8 ((u8)(msg.bufferSize +4));
				bufferSpontaneousMsgForGPU.append (msg.buffer, msg.bufferSize);

				const u8 ck = rhea::utils::simpleChecksum8_calc (&bufferSpontaneousMsgForGPU.buffer[startOfMsg], msg.bufferSize + 3);
				bufferSpontaneousMsgForGPU.appendU8 (ck);
			}
			break;

		default:
			logger->log ("rasPI::MITM::priv_handleIncomingMsgFromThreadQ() => unhandled msg from thread msgQ [what=%d]\n", msg.what);
		}
		
		
		rhea::thread::deleteMsg(msg);
    }
}