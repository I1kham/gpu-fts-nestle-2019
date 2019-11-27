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
							char dumpFileName[256];\
							sprintf_s(dumpFileName, sizeof(dumpFileName), "%s/DUMP_CPUChannelCom.txt", rhea::getPhysicalPathToWritableFolder());\
							rhea::fs::fileDelete(dumpFileName);\
							fDUMP_CPUChannelCom = fopen(dumpFileName, "wt");\
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
	OSSerialPort_setInvalid(comPort);
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
	bool ret = OSSerialPort_open(&comPort, COMPORT, OSSerialPortConfig::Baud115200, false, false, OSSerialPortConfig::Data8, OSSerialPortConfig::NoParity, OSSerialPortConfig::OneStop, OSSerialPortConfig::NoFlowControl);

	if (ret)
		logger->log("OK\n");
	else
		logger->log("FAIL\n");

	logger->decIndent();

	DUMP_OPEN();
	return ret;
}

//*****************************************************************
void CPUChannelCom::close (rhea::ISimpleLogger *logger)
{
	logger->log("CPUChannelCom::close\n");
	OSSerialPort_close(comPort);
		
	DUMP_CLOSE();
}

//*****************************************************************
void CPUChannelCom::closeAndReopen()
{
	OSSerialPort_flushIO(comPort);
	OSSerialPort_close(comPort);
	OSSerialPort_open(&comPort, sCOMPORT, OSSerialPortConfig::Baud115200, false, false, OSSerialPortConfig::Data8, OSSerialPortConfig::NoParity, OSSerialPortConfig::OneStop, OSSerialPortConfig::NoFlowControl);
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
		nBytesSent += OSSerialPort_writeBuffer(comPort, &buffer[nBytesSent], nToWrite);
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
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		//aspetto di riceve un #
		if (nBytesRcv == 0)
		{
			u8 b = 0x00;
			if (0 == OSSerialPort_readBuffer(comPort, &b, 1))
				continue;

			DUMP(&b, 1);

			if (b != (u8)'#')
				continue;
			nBytesRcv++;
		}

		//aspetto di riceve un carattere qualunque subito dopo il #
		if (nBytesRcv == 1)
		{
			if (0 == OSSerialPort_readBuffer(comPort, &commandChar, 1))
				continue;
			nBytesRcv++;

			DUMP(&commandChar, 1);

			bool bFail = false;
			if (commandCharIN == 'B')
			{
				if (commandChar!='B' && commandChar!='Z')
					bFail = true;
			}
			else
			{
				if (commandCharIN != commandChar)
					bFail = true;
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
			if (0 == OSSerialPort_readBuffer(comPort, &msgLen, 1))
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
			const u16 nLetti = (u16)OSSerialPort_readBuffer(comPort, &out_answer[nBytesRcv], nMissing);
			DUMP(&out_answer[nBytesRcv], nLetti);

			nBytesRcv += nLetti;
			if (nBytesRcv >= msgLen)
			{
				DUMPMSG("\n");
				//eval checksum
				if (out_answer[msgLen - 1] == rhea::utils::simpleChecksum8_calc(out_answer, msgLen - 1))
					return true;


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
		if (1 == OSSerialPort_readBuffer(comPort, out_char, 1))
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
		if (0 == OSSerialPort_readBuffer(comPort, &c, 1))
			continue;

		if (c == expectedChar)
			return true;
	}

	return false;
}
