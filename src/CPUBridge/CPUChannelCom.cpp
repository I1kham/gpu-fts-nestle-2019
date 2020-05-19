#include "CPUChannelCom.h"
#include "../rheaCommonLib/rheaUtils.h"

using namespace cpubridge;

//attiva questa define per dumpare su file tutto il traffico lungo la seriale
#define DUMP_COMMUNICATION_TO_FILE


//se non siamo in DEBUG, disabilito il dump d'ufficio
#ifndef _DEBUG
	#ifdef DUMP_COMMUNICATION_TO_FILE
		#undef DUMP_COMMUNICATION_TO_FILE
	#endif
#endif

#ifdef LINUX
    #ifdef DUMP_COMMUNICATION_TO_FILE
        #undef DUMP_COMMUNICATION_TO_FILE
    #endif
#endif

#ifdef DUMP_COMMUNICATION_TO_FILE
	FILE *fDUMP_CPUChannelCom = NULL;

	#define DUMP_OPEN()	{\
							u8 dumpFileName[256];\
							sprintf_s((char*)dumpFileName, sizeof(dumpFileName), "%s/DUMP_CPUChannelCom.txt", rhea::getPhysicalPathToWritableFolder());\
							rhea::fs::fileDelete(dumpFileName);\
							fDUMP_CPUChannelCom = rhea::fs::fileOpenForWriteText(dumpFileName);\
						}\


	#define DUMP_CLOSE()				if (NULL != fDUMP_CPUChannelCom)	{ fclose(fDUMP_CPUChannelCom); fDUMP_CPUChannelCom = NULL; }
	#define DUMP(buffer, lenInBytes)	rhea::utils::dumpBufferInASCII(fDUMP_CPUChannelCom, buffer, lenInBytes);
	#define DUMPMSG(string)				fprintf(fDUMP_CPUChannelCom, string); fflush(fDUMP_CPUChannelCom);
#else
	#define DUMP_OPEN()
	#define DUMP_CLOSE()
	#define DUMP(buffer, lenInBytes)	
	#define DUMPMSG(string)
#endif


//dumpBufferInASCII

//*****************************************************************
CPUChannelCom::CPUChannelCom()
{
	rhea::rs232::setInvalid(comPort);
}

//*****************************************************************
CPUChannelCom::~CPUChannelCom()
{
	DUMP_CLOSE();
}

//*****************************************************************
bool CPUChannelCom::open (const char *COMPORT, rhea::ISimpleLogger *logger)
{
	assert(logger != NULL);

	strcpy_s(sCOMPORT, sizeof(sCOMPORT), COMPORT);

	logger->log ("CPUChannelCom::open\n");
	logger->incIndent();
    bool ret = rhea::rs232::open(&comPort, COMPORT, eRS232BaudRate_115200, false, false, eRS232DataBits_8, eRS232Parity_No, eRS232StopBits_One, eRS232FlowControl_No);

	if (ret)
		logger->log("OK\n");
	else
		logger->log("FAIL\n");

	logger->decIndent();

	//C:/Users/gbrunelli/AppData/Roaming/rheaSMU/DUMP_CPUChannelCom.txt
	DUMP_OPEN();

	return ret;
}

//*****************************************************************
void CPUChannelCom::close (rhea::ISimpleLogger *logger)
{
	logger->log("CPUChannelCom::close\n");
	rhea::rs232::close(comPort);
		
	DUMP_CLOSE();
}

//*****************************************************************
void CPUChannelCom::closeAndReopen()
{
	rhea::rs232::flushIO(comPort);
	rhea::rs232::close(comPort);
    rhea::rs232::open(&comPort, sCOMPORT, eRS232BaudRate_115200, false, false, eRS232DataBits_8, eRS232Parity_No, eRS232StopBits_One, eRS232FlowControl_No);
}


//*****************************************************************
bool CPUChannelCom::sendAndWaitAnswer(const u8 *bufferToSend, u16 nBytesToSend, u8 *out_answer, u16 *in_out_sizeOfAnswer, rhea::ISimpleLogger *logger, u64 timeoutRCVMsec)
{
	if (!priv_handleMsg_send(bufferToSend, nBytesToSend, logger))
		return false;

	const u8 commandChar = bufferToSend[1];
	if (!priv_handleMsg_rcv(commandChar, out_answer, in_out_sizeOfAnswer, logger, timeoutRCVMsec))
		return false;

	return true;
}

/***************************************************
 * priv_handleMsg_send
 *
 *	true se il buffer è stato spedito interamente
 *	false in caso contrario. In ogni caso, entro 2000Msec la fn ritorna false
 */
bool CPUChannelCom::priv_handleMsg_send (const u8 *buffer, u16 nBytesToSend, rhea::ISimpleLogger *logger UNUSED_PARAM)
{
	DUMPMSG("SND: "); DUMP(buffer, nBytesToSend);	DUMPMSG("\n");

	const u64 timeToExitMSec = rhea::getTimeNowMSec() + 2000;
	u16 nBytesSent = 0;
	
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		u32 nToWrite = (nBytesToSend - nBytesSent);
		nBytesSent += rhea::rs232::writeBuffer(comPort, &buffer[nBytesSent], nToWrite);
		if (nBytesSent >= nBytesToSend)
			return true;
	}

	logger->log("CPUChannelCom::priv_handleMsg_send() => unable to send all data. Sent [%d] of [%d]\n", nBytesSent, nBytesToSend);
	DUMPMSG("\nWARN: unable to send all data.\n");
	return false;

}

//***************************************************
bool CPUChannelCom::priv_handleMsg_rcv (u8 commandCharIN, u8 *out_answer, u16 *in_out_sizeOfAnswer, rhea::ISimpleLogger *logger, u64 timeoutRCVMsec)
{
	DUMPMSG("RCV: ");
	const u16 sizeOfBuffer = *in_out_sizeOfAnswer;
	*in_out_sizeOfAnswer = 0;

	const u64 timeToExitMSec = rhea::getTimeNowMSec() + timeoutRCVMsec;
	u16 nBytesRcv = 0;
	u8	commandChar = 0x00;
	u8	msgLen = 0x00;
	bool bIgnoreIf_B_or_Z = false;
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		//aspetto di riceve un #
		if (nBytesRcv == 0)
		{
			u8 b = 0x00;
			if (0 == rhea::rs232::readBuffer(comPort, &b, 1))
				continue;

			DUMP(&b, 1);

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

			DUMP(&commandChar, 1);

			bool bFail = false;
			if (commandCharIN == 'B')
			{
				//se ho inviato un comando "B" (ovvero richiesta di stato), mi aspetto per forza in risposta un comando "B" o "Z"
				if (commandChar!='B' && commandChar!='Z')
					bFail = true;
			}
			else
			{
				//se ho inviato un comando != "B", ovviamente mi aspetto una risposta consona al comando che ho inviato, ma potrebbe anche
				//essere che nel mezzo mi arrivi una risposta ad un comando "B" o "Z".
				//Questo accade perchè il comando "B" purtroppo ha la "proprietà" di poter non ricevere risposte. Se mando "B" e non ricevo risposta,
				//non è un errore. Se non ricevo risposta a 5 "B" consecutivi, allora è un errore.
				//Il problema potrebbe essere che mando un "B", non ricevo risposta (e lo accetto come dato di fatto), poi mando un "X" e mentre aspetto la risposta
				//a "X" ricevo la risposta a "B" che è arrivata molto in ritardo rispetto a quando l'avevo chiesta.
				//Questa situazione fa fallire il comando "X" anche se probabilmente la risposta a "X" arriverà fra poco, dopo la risposta a "B" arrivata in ritardo.
				//Per ovviare a questo problema, se durante l'attesa della risposta di un comando != "B" ricevo una risposta "B", la parso e poi la scarto senza 
				//segnalare errori, vado avanti come se non l'avessi mai ricevuta.
				if (commandCharIN != commandChar)
				{
					if (commandChar == 'B' || commandChar == 'Z')
						bIgnoreIf_B_or_Z = true;
					else
						bFail = true;
				}
			}

			if (bFail == true)
			{
				//non è la risposta che mi aspettavo
				commandChar = 0x00;
				nBytesRcv = 0;
				continue;
			}
		}

		//aspetto di riceve la lunghezza totale del messaggio
		if (nBytesRcv == 2)
		{
			if (0 == rhea::rs232::readBuffer(comPort, &msgLen, 1))
				continue;
			nBytesRcv++;

			DUMP(&msgLen, 1);

			if (sizeOfBuffer < msgLen)
			{
				logger->log("CPUChannelCom::priv_handleMsg_rcv() => ERR answer len is [%d], out_buffer len is only [%d] bytes\n", msgLen, sizeOfBuffer);
				return false;
			}
			if (msgLen < 4)
			{
				logger->log("CPUChannelCom::priv_handleMsg_rcv() => ERR invalid msg len [%d]\n", msgLen);
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
			DUMP(&out_answer[nBytesRcv], nLetti);

			nBytesRcv += nLetti;
			if (nBytesRcv >= msgLen)
			{
				DUMPMSG("\n");
				//eval checksum
				if (out_answer[msgLen - 1] == rhea::utils::simpleChecksum8_calc(out_answer, msgLen - 1))
				{
					//ok, ho in mano una valida risposta della CPU.
					//Dovrei ritornare true e terminare, ma c'è il caso speciale "bIgnoreIf_B_or_Z" da considerare
					if (!bIgnoreIf_B_or_Z)
						return true;

					*in_out_sizeOfAnswer = sizeOfBuffer;
					return priv_handleMsg_rcv (commandCharIN, out_answer, in_out_sizeOfAnswer, logger, timeoutRCVMsec);
				}


				/*HACK: le versioni non beta della CPU calcolano male il CK del msg C
				if (out_answer[1] == 'C')
				{
					logger->log("CPUChannelCom::priv_handleMsg_rcv() => HACK. messaggio C con ck invalida, lo accetto lo stesso\n");
					return true;
				}
				*/


				logger->log("CPUChannelCom::priv_handleMsg_rcv() => ERR, invalid checksum\n", msgLen, sizeOfBuffer);
				DUMPMSG("\nERR, invalid checksum\n");

				return false;
			}
		}
	}

	DUMPMSG("\nERR, timeout rcv\n\n");
	return false;
}

//***************************************************
bool CPUChannelCom::waitChar(u64 timeoutMSec, u8 *out_char)
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

//***************************************************
bool CPUChannelCom::waitForASpecificChar(u8 expectedChar, u64 timeoutMSec)
{
	const u64 timeToExitMSec = rhea::getTimeNowMSec() + timeoutMSec;
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		u8 c;
		if (0 == rhea::rs232::readBuffer(comPort, &c, 1))
			continue;

		if (c == expectedChar)
			return true;
	}

	return false;
}
