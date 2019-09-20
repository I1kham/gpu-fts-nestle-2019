#include "IProtocolChannell.h"
#include "../rhea.h"
#include "../rheaThread.h"

using namespace rhea;



//****************************************************
IProtocolChannell::IProtocolChannell(Allocator *allocatorIN, u32 startingSizeOfReadBuffer)
{
	allocator = allocatorIN; 
	
	nBytesInReadBuffer = 0;
	RBUFFER_SIZE = startingSizeOfReadBuffer;
	rBuffer = (u8*)RHEAALLOC(allocator, RBUFFER_SIZE);
}

//****************************************************
IProtocolChannell::~IProtocolChannell()
{
	RHEAFREE(allocator, rBuffer);
}

//****************************************************
void IProtocolChannell::priv_growReadBuffer()
{
	u32 newReadBufferSize = RBUFFER_SIZE + 1024;
	if (newReadBufferSize < 0xffff)
	{
		u8 *p = (u8*)RHEAALLOC(allocator, newReadBufferSize);
		memcpy(p, rBuffer, RBUFFER_SIZE);
		RBUFFER_SIZE = (u16)newReadBufferSize;

		allocator->dealloc(rBuffer);
		rBuffer = p;
	}
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
		priv_growReadBuffer();
		nMaxToRead = RBUFFER_SIZE - nBytesInReadBuffer;
	}

	assert (nMaxToRead <= RBUFFER_SIZE);
	u16 n = virt_read (&rBuffer[nBytesInReadBuffer], nMaxToRead, timeoutMSec);
	if (n >= protocol::RES_ERROR)
		return n;

	nBytesInReadBuffer += n;
	assert(nBytesInReadBuffer <= RBUFFER_SIZE);
	return nBytesInReadBuffer;
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