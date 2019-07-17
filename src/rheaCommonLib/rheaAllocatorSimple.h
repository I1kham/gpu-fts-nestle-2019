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

        bool                isThreadSafe() const                                            { return true; }

    protected:
        void*               priv_do_alloc (size_t sizeInBytes, size_t align)
        {
            track.onAlloc(sizeInBytes);

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

            return OS_alignedAlloc(align, sizeInBytes);
            //return ::malloc(sizeInBytes);
        }

        void                priv_do_dealloc (const void *p)
                            {
                                track.onDealloc();
								OS_alignedFree((void*)p);
                            }

    private:
        TrackingPolicy      track;
    };



    typedef AllocatorSimple<AllocatorTrackingPolicy_simple> AllocatorSimpleWithMemTrack;
    typedef AllocatorSimple<AllocatorTrackingPolicy_none> AllocatorSimpleNoMemTrack;

} //namespace rhea




#endif // _rheaAllocatorSimple_h_
