#include "CPUChannelCom.h"
#include "../rheaCommonLib/rheaUtils.h"

using namespace cpubridge;


//*****************************************************************
CPUChannelCom::CPUChannelCom()
{
	OSSerialPort_setInvalid(comPort);
}

//*****************************************************************
CPUChannelCom::~CPUChannelCom()
{
}

//*****************************************************************
bool CPUChannelCom::open (const char *COMPORT, rhea::ISimpleLogger *logger)
{
	assert(logger != NULL);

	logger->log ("CPUChannelCom::open\n");
	logger->incIndent();
	bool ret = OSSerialPort_open(&comPort, COMPORT, OSSerialPortConfig::Baud115200, false, false, OSSerialPortConfig::Data8, OSSerialPortConfig::NoParity, OSSerialPortConfig::OneStop, OSSerialPortConfig::NoFlowControl);

	if (ret)
		logger->log("OK\n");
	else
		logger->log("FAIL\n");

	logger->decIndent();
	return ret;
}

//*****************************************************************
void CPUChannelCom::close (rhea::ISimpleLogger *logger)
{
	logger->log("CPUChannelCom::close\n");
	OSSerialPort_close(comPort);
}

//*****************************************************************
bool CPUChannelCom::sendAndWaitAnswer(const u8 *bufferToSend, u16 nBytesToSend, u8 *out_answer, u16 *in_out_sizeOfAnswer, rhea::ISimpleLogger *logger)
{
	if (!priv_handleMsg_send(bufferToSend, nBytesToSend, logger))
		return false;

	if (!priv_handleMsg_rcv(out_answer, in_out_sizeOfAnswer, logger))
		return false;

	return true;
}

/***************************************************
 * priv_handleMsg_send
 *
 *	true se il buffer è stato spedito interamente
 *	false in caso contrario. In ogni caso, entro 2000Msec la fn ritorna false
 */
bool CPUChannelCom::priv_handleMsg_send (const u8 *buffer, u16 nBytesToSend, rhea::ISimpleLogger *logger)
{
	const u64 timeToExitMSec = rhea::getTimeNowMSec() + 2000;
	u16 nBytesSent = 0;
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		u32 nToWrite = (nBytesToSend - nBytesSent);
		nBytesSent += OSSerialPort_writeBuffer(comPort, &buffer[nBytesSent], nToWrite);
		if (nBytesSent >= nBytesToSend)
			return true;
	}

	return false;
}

//***************************************************
bool CPUChannelCom::priv_handleMsg_rcv (u8 *out_answer, u16 *in_out_sizeOfAnswer, rhea::ISimpleLogger *logger)
{
	const u16 sizeOfBuffer = *in_out_sizeOfAnswer;
	*in_out_sizeOfAnswer = 0;

	const u64 timeToExitMSec = rhea::getTimeNowMSec() + 2000;
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
		}

		//aspetto di riceve la lunghezza totale del messaggio
		if (nBytesRcv == 2)
		{
			if (0 == OSSerialPort_readBuffer(comPort, &msgLen, 1))
				continue;
			nBytesRcv++;

			if (sizeOfBuffer < msgLen)
			{
				logger->log("CPUChannelCom::priv_handleMsg_rcv() => ERR answer len is [%d], out_buffer len is only [%d] bytes\n", msgLen, sizeOfBuffer);
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
			nBytesRcv += OSSerialPort_readBuffer(comPort, &out_answer[nBytesRcv], nMissing);
			if (nBytesRcv >= msgLen)
			{
				//eval checksum
				if (out_answer[msgLen - 1] == rhea::utils::simpleChecksum8_calc(out_answer, msgLen - 1))
					return true;

				logger->log("CPUChannelCom::priv_handleMsg_rcv() => ERR, invalid checksum\n", msgLen, sizeOfBuffer);
				return false;
			}
		}
	}
	return false;
}

