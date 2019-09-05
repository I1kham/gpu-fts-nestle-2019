#include "IProtocol.h"

using namespace rhea;

//*************************************************
IProtocol::IProtocol (Allocator * allocatorIN, u16 sizeOfWriteBufferIN)
{
	allocator = allocatorIN;
	BUFFERW_SIZE = sizeOfWriteBufferIN;
	bufferW = (u8*)RHEAALLOC(allocator, BUFFERW_SIZE);
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
	u16 nBytesInReadBuffer = ch->read (timeoutMSec);
	if (nBytesInReadBuffer >= protocol::RES_ERROR)
		return nBytesInReadBuffer;

	while (1)
	{
		if (ch->getNumBytesInReadBuffer() == 0)
			return 0;

		u16 nBytesInseritiInOutResult = 0;
		u16 nBytesConsumati = virt_decodeBuffer (ch, ch->getReadBuffer(), ch->getNumBytesInReadBuffer(), out_result, &nBytesInseritiInOutResult);

		if (nBytesConsumati == 0)
			return 0;
		if (nBytesConsumati >= protocol::RES_ERROR)
			return nBytesConsumati;

		ch->consumeReadBuffer (nBytesConsumati);
		if (nBytesInseritiInOutResult)
			return nBytesInseritiInOutResult;
	}
}


//*************************************************
u16 IProtocol::write (IProtocolChannell *ch, const u8 *bufferToWrite, u16 nBytesToWrite, u32 timeoutMSec)
{
	if (nBytesToWrite == 0 || NULL == bufferToWrite)
		return 0;

	u16 nBytesToSend = virt_encodeBuffer (bufferToWrite, nBytesToWrite, bufferW, BUFFERW_SIZE);
	if (nBytesToSend >= protocol::RES_ERROR)
		return nBytesToSend;

	if (nBytesToSend > 0)
		return ch->write(bufferW, nBytesToSend, timeoutMSec);
	return nBytesToSend;
}