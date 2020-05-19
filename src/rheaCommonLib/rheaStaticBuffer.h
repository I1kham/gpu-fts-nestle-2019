#ifndef _rheaStaticBuffer_h_
#define _rheaStaticBuffer_h_
#include "rheaMemory.h"
#include "OS/OS.h"

namespace rhea
{
	/******************************************************************************
	 * StaticBuffer
	 *
	 * Gestisce un blocco di memoria fornito esternamente. Il blocco non cresce mai, e non viene freed
	 *******************************************************************************/
	class StaticBuffer
	{
	public:
						StaticBuffer()																							{ mem = NULL; allocatedSize = 0; }

		void			setup (void *memBlock, u32 sizeOfMemBlock)																{ mem = (u8*)memBlock; allocatedSize = sizeOfMemBlock; }

		bool			read  (void *dest, u32 offset, u32 nBytesToread) const;
		bool			write (const void *src, u32 offset, u32 nBytesTowrite, bool bCangrow=false);
        bool			growIncremental (u32 howManyBytesToAdd UNUSED_PARAM)													{ return false; }
		bool			growUpTo (u32 finalSize)																				{ return (finalSize <= allocatedSize); }
		u32				getTotalSizeAllocated() const																			{ return allocatedSize; }

	private:
						RHEA_NO_COPY_NO_ASSIGN(StaticBuffer);

	private:
		u8				*mem;
		u32				allocatedSize;
	};

	/******************************************************************************
	 * StaticBufferReadOnly
	 *
	 * Gestisce un blocco di memoria fornito esternamente. Il blocco non cresce mai, e non viene freed
	 *	Questa particolare istanza inolte non consente la scrittura
	 *******************************************************************************/
	class StaticBufferReadOnly
	{
	public:
						StaticBufferReadOnly()																			{ mem = NULL; allocatedSize = 0; }

		void			setup(const void *memBlock, u32 sizeOfMemBlock)													{ mem = (u8*)memBlock; allocatedSize = sizeOfMemBlock; }

		bool			read(void *dest, u32 offset, u32 nBytesToread) const;
		bool			write (const void *src, u32 offset, u32 nBytesTowrite, bool bCangrow = false);
        bool			growIncremental(u32 howManyBytesToAdd UNUSED_PARAM)												{ return false; }
		bool			growUpTo(u32 finalSize)																			{ return (finalSize <= allocatedSize); }
		u32				getTotalSizeAllocated() const																	{ return allocatedSize; }

	private:
						RHEA_NO_COPY_NO_ASSIGN(StaticBufferReadOnly);

	private:
		const u8		*mem;
		u32				allocatedSize;
	};
}; //namespace rhea
#endif //_rheaStaticBuffer_h_
