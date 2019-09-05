#include "ProtocolChSocketTCP.h"

using namespace rhea;


//**************************************************
u16 ProtocolChSocketTCP::virt_read (u8 *buffer, u16 nMaxBytesToRead, u32 timeoutMSec)
{
	i32 nBytesLetti = OSSocket_read (sok, buffer, nMaxBytesToRead, timeoutMSec);
	if (nBytesLetti == 0)
		return protocol::RES_CHANNEL_CLOSED;

	if (nBytesLetti < 0)
		return 0;

	assert(nBytesLetti <= nMaxBytesToRead);
	return (u16)nBytesLetti;
}

//**************************************************
u16 ProtocolChSocketTCP::virt_write (const u8 *bufferToWrite, u16 nBytesToWrite)
{
	i32 n = OSSocket_write (sok, bufferToWrite, nBytesToWrite);
	if (n < 0)
		return protocol::RES_CHANNEL_CLOSED;
	return (u16)n;
}