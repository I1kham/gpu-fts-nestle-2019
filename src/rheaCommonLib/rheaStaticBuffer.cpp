#include "rheaStaticBuffer.h"


using namespace rhea;


//******************************************
bool StaticBuffer::read  (void *dest, u32 offset, u32 nBytesToread) const
{
	if (nBytesToread == 0)
		return true;
	if (offset + nBytesToread > allocatedSize)
		return false;
	memcpy (dest, &mem[offset], nBytesToread);
	return true;
}

//******************************************
bool StaticBuffer::write  (const void *src, u32 offset, u32 nBytesTowrite, bool bCangrow)
{
	if (nBytesTowrite == 0)
		return true;
	if (offset + nBytesTowrite > allocatedSize)
		return false;
	memcpy (&mem[offset], src, nBytesTowrite);
	return true;
}




//******************************************
bool StaticBufferReadOnly::read (void *dest, u32 offset, u32 nBytesToread) const
{
	if (nBytesToread == 0)
		return true;
	if (offset + nBytesToread > allocatedSize)
		return false;
	memcpy(dest, &mem[offset], nBytesToread);
	return true;
}

//******************************************
bool StaticBufferReadOnly::write(const void *src UNUSED_PARAM, u32 offset UNUSED_PARAM, u32 nBytesTowrite UNUSED_PARAM, bool bCangrow UNUSED_PARAM)
{
	//questo static buffer è readonly!!
	DBGBREAK;
	return false;
}