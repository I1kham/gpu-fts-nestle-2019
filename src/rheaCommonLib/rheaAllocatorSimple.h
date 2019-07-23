#ifndef _rheaAllocatorSimple_h_
#define _rheaAllocatorSimple_h_
#include <stddef.h>
#include <stdlib.h>
#include "rheaMemory.h"
#include "rheaAllocatorTrackingPolicy.h"

namespace rhea
{
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
    public:
                            AllocatorSimple(const char *nameIN)  : Allocator(nameIN)          { }
        virtual             ~AllocatorSimple()
                            {
                                assert(!track.anyMemLeaks());
                            }

        static u8           getSizeOfSize_t()                                               { if (sizeof(size_t) > 4) return 8; return 4; }


        bool                isThreadSafe() const                                            { return true; }
        size_t              getAllocatedSize (const void *p) const
                            {
                                const u8 *pp = (const u8*)p;
                                pp -= getSizeOfSize_t();

                                const size_t *p2 = (const size_t*)pp;
                                return p2[0];
                            }

    protected:
        void*               priv_do_alloc (size_t sizeInBytes, size_t align)
                            {
                                //alloco 4 o 8 byte in più per memorizzare il size di allocazione
                                sizeInBytes += getSizeOfSize_t();

                                switch (align)
                                {
                                case 2:
                                case 4:
                                case 8:
                                case 16:
                                    break;

                                default:
                                    //align = __alignof(max_align_t);
                                    align = __alignof(void*);
                                    break;
                                }


                                //sizeInBytes deve essere un multiplo di align
                                size_t m = (sizeInBytes % align);
                                if (m)
                                    sizeInBytes += (align - m);

                                u8 *p =  (u8*)OS_alignedAlloc(align, sizeInBytes);

                                track.onAlloc(sizeInBytes);

                                memcpy (p, &sizeInBytes, sizeof(size_t));
                                p += getSizeOfSize_t();

                                return (void*)p;
                            }

        void                priv_do_dealloc (const void *p)
                            {
                                track.onDealloc(getAllocatedSize(p));
                                const u8 *pp = (const u8*)p;
                                pp -= getSizeOfSize_t();
                                OS_alignedFree((void*)pp);
                            }

    private:
        TrackingPolicy      track;
    };



    typedef AllocatorSimple<AllocatorTrackingPolicy_simple> AllocatorSimpleWithMemTrack;
    typedef AllocatorSimple<AllocatorTrackingPolicy_none> AllocatorSimpleNoMemTrack;

} //namespace rhea




#endif // _rheaAllocatorSimple_h_
