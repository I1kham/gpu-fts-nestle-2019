#include "IProtocol.h"

using namespace rhea;

//*************************************************
IProtocol::IProtocol (Allocator * allocatorIN, u16 startingSizeOfWriteBuffer, u16 maxSizeOfWriteBuffer)
{
	allocator = allocatorIN;
	BUFFERW_CUR_SIZE = startingSizeOfWriteBuffer;
	BUFFERW_MAX_SIZE = maxSizeOfWriteBuffer;
	bufferW = (u8*)RHEAALLOC(allocator, BUFFERW_CUR_SIZE);
}

//*************************************************
IProtocol::~IProtocol()
{
	RHEAFREE(allocator, bufferW);
	bufferW = NULL;
}

//*************************************************
u16 IProtocol::read (IProtocolChannell *ch, u32 timeoutMSec, LinearBuffer &out_result)
{
	//legge dal canale di comunicazione e bufferizza
	u16 err = ch->read(timeoutMSec);
	if (err >= protocol::RES_ERROR)
		return err;

	u16 offset = 0;
	while (1)
	{
		//prova a parsare il buffer
		u16 nBytesInBuffer = ch->getNumBytesInReadBuffer();
		if (nBytesInBuffer == 0)
			return offset;

		u16 nBytesInseritiInOutResult = 0;
		u16 nBytesConsumati = virt_decodeBuffer(ch, ch->getReadBuffer(), ch->getNumBytesInReadBuffer(), out_result, offset, &nBytesInseritiInOutResult);
		if (nBytesConsumati == 0)
			return offset;
		if (nBytesConsumati >= protocol::RES_ERROR)
			return nBytesConsumati;

		ch->consumeReadBuffer(nBytesConsumati);
		if (nBytesInseritiInOutResult)
			offset += nBytesInseritiInOutResult;

		err = ch->read(0);
		if (err >= protocol::RES_ERROR)
			return err;

	}
}


//****************************************************
bool IProtocol::priv_growWriteBuffer()
{
	if (BUFFERW_CUR_SIZE < BUFFERW_MAX_SIZE)
	{
		u32 newBufferSize = BUFFERW_CUR_SIZE * 2;
		if (newBufferSize < 0xffff)
		{
			u8 *p = (u8*)RHEAALLOC(allocator, newBufferSize);
			memcpy(p, bufferW, BUFFERW_CUR_SIZE);
			BUFFERW_CUR_SIZE = (u16)newBufferSize;

			allocator->dealloc(bufferW);
			bufferW = p;
			return true;
		}
	}
	return false;
}

//*************************************************
u16 IProtocol::write (IProtocolChannell *ch, const u8 *bufferToWrite, u16 nBytesToWrite, u32 timeoutMSec)
{
	if (nBytesToWrite == 0 || NULL == bufferToWrite)
		return 0;

	u16 nBytesToSend = virt_encodeBuffer (bufferToWrite, nBytesToWrite, bufferW, BUFFERW_CUR_SIZE);

	if (nBytesToSend == protocol::RES_PROTOCOL_WRITEBUFFER_TOOSMALL)
	{
		if (!priv_growWriteBuffer())
			return nBytesToSend;
		return write(ch, bufferToWrite, nBytesToWrite, timeoutMSec);
	}

	if (nBytesToSend >= protocol::RES_ERROR)
		return nBytesToSend;

	if (nBytesToSend > 0)
		return ch->write(bufferW, nBytesToSend, timeoutMSec);
	return nBytesToSend;
}