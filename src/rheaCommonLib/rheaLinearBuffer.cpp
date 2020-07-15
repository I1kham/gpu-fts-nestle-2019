#include "rheaLinearBuffer.h"
#include "rhea.h"

using namespace rhea;


//******************************************
LinearBuffer::LinearBuffer() :
    allocator (NULL)
{
    mem = NULL;
    bFreeMemBlock = 0;
    allocatedSize = 0;
}

//******************************************
LinearBuffer::~LinearBuffer()
{
    priv_FreeCurBuffer ();
}

//******************************************
void LinearBuffer::priv_FreeCurBuffer ()
{
    if (bFreeMemBlock)
        allocator->dealloc (mem);
}

//******************************************
void LinearBuffer::setupWithBase (void *startingBlock, u32 sizeOfStartingBlock, rhea::Allocator *backingallocator)
{
    assert (NULL == mem);
    allocator = backingallocator;
    mem = (u8*)startingBlock;
    bFreeMemBlock = 0;
    allocatedSize = sizeOfStartingBlock;
}

//******************************************
void LinearBuffer::setup (rhea::Allocator *backingallocator, u32 preallocNumBytes)
{
    assert (NULL == mem);
    allocator = backingallocator;

    bFreeMemBlock = 1;

    if (preallocNumBytes)
    {
        mem = RHEAALLOCT(u8*,allocator,preallocNumBytes);
        allocatedSize = preallocNumBytes;
    }
    else
    {
        mem = NULL;
        allocatedSize = 0;
    }
}

//******************************************
bool LinearBuffer::read  (void *dest, u32 offset, u32 nBytesToread) const
{
    if (nBytesToread == 0)
        return true;
    if (offset + nBytesToread > allocatedSize)
        return false;
    memcpy (dest, &mem[offset], nBytesToread);
    return true;
}

//******************************************
bool LinearBuffer::write  (const void *src, u32 offset, u32 nBytesTowrite, bool bCangrow)
{
    if (nBytesTowrite == 0)
        return true;
    if (offset + nBytesTowrite > allocatedSize)
    {
        if (!bCangrow)
            return false;
        if (!growUpTo (offset + nBytesTowrite))
            return false;
    }
    memcpy (&mem[offset], src, nBytesTowrite);
    return true;
}

//******************************************
bool LinearBuffer::growUpTo (u32 finalSize)
{
    if (allocatedSize >= finalSize)
        return true;
    return growIncremental (finalSize - allocatedSize);
}

//******************************************
bool LinearBuffer::growIncremental (u32 howManyBytesToAdd)
{
    if (howManyBytesToAdd == 0)
        return true;
    u8 *newBuffer = RHEAALLOCT(u8*,allocator, allocatedSize + howManyBytesToAdd);
    if (NULL == newBuffer)
    {
        DBGBREAK;
        return false;
    }

    if (allocatedSize)
        memcpy (newBuffer, mem, allocatedSize);

    priv_FreeCurBuffer();
    mem = newBuffer;
    bFreeMemBlock = 1;
    allocatedSize += howManyBytesToAdd;
    return true;
}

//******************************************
bool LinearBuffer::copyFrom (const LinearBuffer &src, u32 srcOffset, u32 nBytesToCopy, u32 dstOffset, bool bCangrow)
{
    if (nBytesToCopy == 0)
        return true;
    assert (srcOffset < src.getTotalSizeAllocated());
    assert (srcOffset + nBytesToCopy <= src.getTotalSizeAllocated());

    assert (dstOffset < allocatedSize);

    if (dstOffset + nBytesToCopy > allocatedSize)
    {
        if (!bCangrow)
            return false;
        if (!growUpTo (dstOffset + nBytesToCopy))
            return false;
    }

    memcpy (&mem[dstOffset], &src.mem[srcOffset], nBytesToCopy);
    return true;
}

//******************************************
void LinearBuffer::shrink (u32 newSize, rhea::Allocator *newAllocator)
{
    assert (newSize && newSize <= allocatedSize);

    if (newAllocator == NULL)
        newAllocator = allocator;

    u8	*temp = RHEAALLOCT(u8*,newAllocator, newSize);
    memcpy (temp, mem, newSize);

    unsetup ();
    allocatedSize = newSize;
    bFreeMemBlock = 1;
    mem = temp;
    allocator = newAllocator;
}
