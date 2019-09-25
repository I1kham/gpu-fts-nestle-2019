#include "IProtocolChannell.h"
#include "../rhea.h"
#include "../rheaThread.h"

using namespace rhea;



//****************************************************
IProtocolChannell::IProtocolChannell(Allocator *allocatorIN, u16 startingSizeOfReadBufferInBytes, u16 maxSizeOfReadBufferInBytes)
{
	allocator = allocatorIN; 
	
	nBytesInReadBuffer = 0;
	RBUFFER_SIZE = startingSizeOfReadBufferInBytes;
	MAX_RBUFFER_SIZE = maxSizeOfReadBufferInBytes;
	rBuffer = (u8*)RHEAALLOC(allocator, RBUFFER_SIZE);
}

//****************************************************
IProtocolChannell::~IProtocolChannell()
{
	RHEAFREE(allocator, rBuffer);
}

//****************************************************
bool IProtocolChannell::priv_growReadBuffer()
{
	if (RBUFFER_SIZE < MAX_RBUFFER_SIZE)
	{
		u32 newReadBufferSize = RBUFFER_SIZE * 2;
		if (newReadBufferSize < 0xffff)
		{
			u8 *p = (u8*)RHEAALLOC(allocator, newReadBufferSize);
			memcpy(p, rBuffer, RBUFFER_SIZE);
			RBUFFER_SIZE = (u16)newReadBufferSize;

			allocator->dealloc(rBuffer);
			rBuffer = p;
			return true;
		}
	}
	return false;
}

//****************************************************
void IProtocolChannell::consumeReadBuffer (u16 nBytesConsumati)
{
	assert (nBytesInReadBuffer >= nBytesConsumati);
	nBytesInReadBuffer -= nBytesConsumati;
	if (nBytesInReadBuffer > 0)
	{
		assert(nBytesConsumati + nBytesInReadBuffer <= RBUFFER_SIZE);
		memcpy(rBuffer, &rBuffer[nBytesConsumati], nBytesInReadBuffer);
	}
}

//******************************************************
u16 IProtocolChannell::read (u32 timeoutMSec)
{
	assert (nBytesInReadBuffer <= RBUFFER_SIZE);
	u32 nMaxToRead = RBUFFER_SIZE - nBytesInReadBuffer;
	if (nMaxToRead == 0)
	{
		if (!priv_growReadBuffer())
			return 0;
		nMaxToRead = RBUFFER_SIZE - nBytesInReadBuffer;
	}

	assert (nBytesInReadBuffer + nMaxToRead <= RBUFFER_SIZE);
	u16 n = virt_read (&rBuffer[nBytesInReadBuffer], nMaxToRead, timeoutMSec);
	if (n >= protocol::RES_ERROR)
		return n;

	assert(n <= nMaxToRead);
	nBytesInReadBuffer += n;
	assert(nBytesInReadBuffer <= RBUFFER_SIZE);
	return n;
}

//******************************************************
u16 IProtocolChannell::write (const u8 *bufferToWrite, u16 nBytesToWrite, u32 timeoutMSec)
{ 
	const u64 timeToExitMSec = rhea::getTimeNowMSec() + timeoutMSec;
	
	u16 nWrittenSoFar = virt_write (bufferToWrite, nBytesToWrite);
	
	//if (nWrittenSoFar >= PROTORES_ERROR) return nWrittenSoFar;
	if (nWrittenSoFar >= nBytesToWrite) // l'if qui sopra non serve perchè in caso di errore, sicuramente nWrittenSoFar è >= di nBytesToWrite e quindi questa
		return nWrittenSoFar;			// condizione da sola le copre entrambe

	do
	{
		rhea::thread::sleepMSec (50);

		u16 n = virt_write (&bufferToWrite[nWrittenSoFar], (nBytesToWrite - nWrittenSoFar));
		if (n >= protocol::RES_ERROR)
			return n;

		if (n > 0)
		{
			nWrittenSoFar += n;
			if (nWrittenSoFar >= nBytesToWrite)
				return nWrittenSoFar;
		}
	
	} while (rhea::getTimeNowMSec() < timeToExitMSec);

	return nWrittenSoFar;
}