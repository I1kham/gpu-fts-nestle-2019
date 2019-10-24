#ifndef _rheaAllocatorSimple_h_
#define _rheaAllocatorSimple_h_
#include <stddef.h>
#include <stdlib.h>
#include "rheaMemory.h"
#include "rheaMemoryTracker.h"
#include "rheaAllocatorTrackingPolicy.h"

namespace rhea
{
#ifdef _DEBUG
	#define ALLOCATOR_SIMPLE_USE_SAFE_GUARD
    #undef ALLOCATOR_SIMPLE_USE_MEM_TRACKER
#endif

#ifdef ALLOCATOR_SIMPLE_USE_SAFE_GUARD
	#define ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE	(8*128)
#endif

#ifdef ALLOCATOR_SIMPLE_USE_MEM_TRACKER
	#define MEMTRACKER_ON_ALLOC(p,allocatedSizeInByte, debug_filename, debug_lineNumber)		rhea::internal_getMemoryTracker()->onAlloc(getAllocatorID(), getName(), p, allocatedSizeInByte, debug_filename, debug_lineNumber);
	#define MEMTRACKER_ON_DEALLOC(p_original, allocated_size)									rhea::internal_getMemoryTracker()->onDealloc(getAllocatorID(), getName(), p_original, allocated_size);
#else
	#define MEMTRACKER_ON_ALLOC(p,allocatedSizeInByte, debug_filename, debug_lineNumber)		
	#define MEMTRACKER_ON_DEALLOC(p_original, allocated_size)
#endif


    /*************************************************************************
     * AllocatorSimple
     *
     * Allocator di default, semplice, thread safe che usa la aligned_malloc
     *
     *
     */
    template<typename TrackingPolicy>
    class AllocatorSimple : public Allocator
    {
	private:
		static const u8 NUM_OF_RESERVED_BYTES = 8;

    public:
							AllocatorSimple(const char *nameIN) : Allocator(nameIN)
							{ 
							}


        virtual             ~AllocatorSimple()												{ assert(!track.anyMemLeaks()); }

        bool                isThreadSafe() const                                            { return true; }
		size_t              getAllocatedSize (const void *p) const							
							{ 
								const u8 *pp = (const u8*)p;
								pp -= 8;
								//const u16 allocatorID = ((u16)(pp[2]) << 8) | (u16)(pp[3]);
								const u32 size_and_align = (((u32)pp[4]) << 24) | (((u32)pp[5]) << 16) | (((u32)pp[6]) << 8) | (((u32)pp[7]));
								return (size_and_align & 0x3fffffff);
							}

    protected:
							//*************************************************************
        void*               virt_do_alloc(size_t sizeInBytes, size_t align, const char *debug_filename, u32 debug_lineNumber, bool bPlacementNew)
                            {
								assert(priv_validate_all_pointers());

								//alloco allineato a 4 oppure 8
								assert(align <= 8);
								u8 real_align;
								if (align <= 4)
									real_align = 4;
								else
									real_align = 8;

								//calcolo la dimensione reale del blocco da allocare
								u32 real_size_to_alloc = priv_calcRealSizeToAlloc (sizeInBytes, real_align);

								//alloco
								u8 *p_original = (u8*)OS_alignedAlloc(real_align, real_size_to_alloc);
								assert(p_original);
								assert(IS_POINTER_ALIGNED(p_original, real_align));
								track.onAlloc(real_size_to_alloc);

								u8 *ret = p_original;

#ifdef ALLOCATOR_SIMPLE_USE_SAFE_GUARD
								//safe guard ad inizio blocco
								priv_putSafeGuardHere(p_original);

								//safe guard a fine blocco
								priv_putSafeGuardHere(p_original + real_size_to_alloc - ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE);

								ret += ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE;
#endif

								//memorizzo allocatorID
								ret[2] = (u8)((getAllocatorID() & 0xFF00) >> 8);
								ret[3] = (u8)(getAllocatorID() & 0x00FF);


								//memorizzo il size allocato e setto un bit per indicare se allineamento 4 o 8 byte
								u32 size_and_align = real_size_to_alloc;
								if (real_align == 8)
									size_and_align |= 0x80000000;

								//uso un bit anche per indicare se placementNew
								if (bPlacementNew)
									size_and_align |= 0x40000000;

								ret[4] = (((u32)size_and_align & 0xFF000000) >> 24);
								ret[5] = (((u32)size_and_align & 0x00FF0000) >> 16);
								ret[6] = (((u32)size_and_align & 0x0000FF00) >> 8);
								ret[7] = (((u32)size_and_align & 0x000000FF));

								ret += 8;

								MEMTRACKER_ON_ALLOC(p_original, real_size_to_alloc, debug_filename, debug_lineNumber);

								assert(priv_validate_all_pointers());
                                return ret;
                            }

							//*************************************************************
        void                virt_do_dealloc (void *p, bool bPlacementNew)
                            {
#ifdef ALLOCATOR_SIMPLE_USE_SAFE_GUARD
								assert(priv_validate_all_pointers());
								priv_check_safeGuards(p);
#endif

								u8 *p_original = (u8*)p;
								p_original -= 8;
								
								//recupero allocator ID, size allocated, align e placement
								const u16 allocatorID = ((u16)(p_original[2]) << 8) | (u16)(p_original[3]);
								const u32 size_and_align = (((u32)p_original[4]) << 24) | (((u32)p_original[5]) << 16) | (((u32)p_original[6]) << 8) | (((u32)p_original[7]));
								const u32 allocated_size = size_and_align & 0x3fffffff;

								assert(getAllocatorID() == allocatorID);
								assert(( bPlacementNew && (size_and_align & 0x40000000) != 0) ||
									   (!bPlacementNew && (size_and_align & 0x40000000) == 0));

#ifdef ALLOCATOR_SIMPLE_USE_SAFE_GUARD
								//memsetto l'intero buffer per cancellare ogni traccia di quello che c'era prima
								p_original -= ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE;
								memset (p_original, 0xAC, allocated_size);
#endif		

								MEMTRACKER_ON_DEALLOC(p_original, allocated_size);

								track.onDealloc(allocated_size);
								OS_alignedFree(p_original);
                            }

	private:
							//*************************************************************
		u32					priv_calcRealSizeToAlloc(size_t sizeInBytes, u8 real_align) const
							{
								assert (real_align == 4 || real_align == 8);

								//la qtà di memoria da allocare deve essere un multiplo dell'allineamento
								u32 real_size_to_alloc = ALIGN_NUMEBER_TO_POWER_OF_TWO((u32)sizeInBytes, real_align);

								//Voglio memorizzare anche il sizeInBytes, mi servono quindi altri 4 byte oltre a quelli richiesti dall'utente
								//Uso 30 bit per memorizzare il size e 1 bit (MSB) per indicare se ho allineato a 4 oppure a 8 e un'altro bit
								//per indicare se è un placementNew oppure no.
								//Voglio memorizzare anche l'allocatordID, quindi altri 2 byte.
								//In tutto quindi, mi riservo 8 byte
								assert(sizeInBytes <= 0x3fffffff);
								assert(NUM_OF_RESERVED_BYTES % real_align == 0);
								real_size_to_alloc += NUM_OF_RESERVED_BYTES;


					#ifdef ALLOCATOR_SIMPLE_USE_SAFE_GUARD
								//alloco delle safe guard prima e dopo il blocco
								assert(ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE % real_align == 0);
								real_size_to_alloc += ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE;
								real_size_to_alloc += ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE;
					#endif
								return real_size_to_alloc;
							}


#ifdef ALLOCATOR_SIMPLE_USE_SAFE_GUARD
		void				priv_putSafeGuardHere (u8 *p) const
							{
								assert(ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE % 4 == 0);
								for (u16 i = 0; i < ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE;)
								{
									p[i++] = 0xAC;
									p[i++] = 0xDC;
									p[i++] = 0x17;
									p[i++] = 0x08;
								}
							}

		bool				priv_check_safeGuards(const void *p) const 
							{
								//Calcolo il p originalmente allocato
								u8 *p_original = (u8*)p;
								p_original -= NUM_OF_RESERVED_BYTES;
								p_original -= ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE;

								//check safe guard ad inizio blocco
								if (!priv_check_singleSafeGuard(p_original))
									return false;
								p_original += ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE;

								//recupero info riguardo alla dimensione allocata
								const u32 size_and_align = (((u32)p_original[4]) << 24) | (((u32)p_original[5]) << 16) | (((u32)p_original[6]) << 8) | (((u32)p_original[7]));
								const u32 real_size_allocated = (size_and_align & 0x3FFFFFFF);

								//calcolo il totale dei bytes che è stato allocato
								p_original -= ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE;
								p_original += real_size_allocated;
								p_original -= ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE;
								return priv_check_singleSafeGuard(p_original);
							}

		bool				priv_check_singleSafeGuard(const u8 *p) const
							{
								assert(ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE % 4 == 0);
								for (u16 i = 0; i < ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE; i+=4)
								{
									bool b  = (	p[i + 0] == 0xAC &&
												p[i + 1] == 0xDC &&
												p[i + 2] == 0x17 &&
												p[i + 3] == 0x08);
									if (!b)
										return false;
								}
								return true;
							}


		bool				priv_validate_all_pointers() const
							{
	#if (defined(ALLOCATOR_SIMPLE_USE_SAFE_GUARD) && defined(ALLOCATOR_SIMPLE_USE_MEM_TRACKER))
								/*MemoryTracker::sRecord *s = rhea::internal_getMemoryTracker()->getRoot();
								while (s)
								{
									if (s->allocatorID == getAllocatorID() && (s->allocID & 0x80000000) == 0)
									{
										u8 *p_returned = (u8*)s->p;
										p_returned += NUM_OF_RESERVED_BYTES;
										p_returned += ALLOCATOR_SIMPLE_USE_SAFE_GUARD_SIZE;
										if (!priv_check_safeGuards(p_returned))
										{
											DBGBREAK;
											return false;
										}
									}
									s = s->next;
								}
								*/
								return true;
	#else
								return true;
	#endif //#if defined(ALLOCATOR_SIMPLE_USE_SAFE_GUARD) && defined(ALLOCATOR_SIMPLE_USE_MEM_TRACKER)
							}

#endif //ALLOCATOR_SIMPLE_USE_SAFE_GUARD
	private:
        TrackingPolicy      track;
    };



    typedef AllocatorSimple<AllocatorTrackingPolicy_simple> AllocatorSimpleWithMemTrack;
    typedef AllocatorSimple<AllocatorTrackingPolicy_none> AllocatorSimpleNoMemTrack;

} //namespace rhea




#endif // _rheaAllocatorSimple_h_
