#include "ProtocolChSocketTCP.h"

using namespace rhea;

#ifdef DUMP_CProtocolChSocketTCP_TO_FILE
	#include "../rhea.h"
	#include "../rheaUtils.h"
	u16			ProtocolChSocketTCP::dump_nextDumpFileID = 0;

	#define DUMP_CLOSE()				if (NULL != fDUMP)	{ fclose(fDUMP); fDUMP = NULL; }
	#define DUMP(buffer, lenInBytes)	rhea::utils::dumpBufferInASCII(fDUMP, buffer, lenInBytes);
	#define DUMPMSG(string)				fprintf(fDUMP, string); fflush(fDUMP);
#else
	#define DUMP_CLOSE()
	#define DUMP(buffer, lenInBytes)	
	#define DUMPMSG(string)
#endif


//**************************************************
ProtocolChSocketTCP::ProtocolChSocketTCP(Allocator *allocatorIN, u16 startingSizeOfReadBufferInBytes, u16 maxSizeOfReadBufferInBytes) :
	IProtocolChannell(allocatorIN, startingSizeOfReadBufferInBytes, maxSizeOfReadBufferInBytes)
{
	OSSocket_init(&sok);

#ifdef DUMP_CProtocolChSocketTCP_TO_FILE
	char dumpFileName[256];
	sprintf_s(dumpFileName, sizeof(dumpFileName), "%s/DUMP_ProtocolChSocketTCP_%03d.txt", rhea::getPhysicalPathToWritableFolder(), dump_nextDumpFileID++);
	rhea::deleteFile(dumpFileName);
	fDUMP = fopen(dumpFileName, "wt");
#endif
}

//**************************************************
ProtocolChSocketTCP::~ProtocolChSocketTCP() 
{
	DUMP_CLOSE();
}




//**************************************************
u16 ProtocolChSocketTCP::virt_read (u8 *buffer, u16 nMaxBytesToRead, u32 timeoutMSec)
{
	i32 nBytesLetti = OSSocket_read (sok, buffer, nMaxBytesToRead, timeoutMSec);
	if (nBytesLetti == 0)
		return protocol::RES_CHANNEL_CLOSED;

	if (nBytesLetti < 0)
		return 0;

	assert(nBytesLetti <= nMaxBytesToRead);
	DUMPMSG("RCV: "); DUMP(buffer, nBytesLetti); DUMPMSG("\n");
	return (u16)nBytesLetti;
}

//**************************************************
u16 ProtocolChSocketTCP::virt_write (const u8 *bufferToWrite, u16 nBytesToWrite)
{
	i32 n = OSSocket_write (sok, bufferToWrite, nBytesToWrite);
	if (n < 0)
		return protocol::RES_CHANNEL_CLOSED;

	DUMPMSG("SND: "); DUMP(bufferToWrite, n); DUMPMSG("\n");
	return (u16)n;
}