#ifndef _rheaAllocatorTrackingPolicy_h_
#define _rheaAllocatorTrackingPolicy_h_
#include "OS/OS.h"

namespace rhea
{
    /***********************************************************************
    * AllocatorTrackingPolicy_none
    *
    *
    ***********************************************************************/
    class AllocatorTrackingPolicy_none
    {
    public:
                        AllocatorTrackingPolicy_none ()                                 { }

        bool			anyMemLeaks()                                                   { return false; }
        void			onAlloc (size_t size UNUSED_PARAM)                              { }
        void			onDealloc (size_t size UNUSED_PARAM)                            { }

    private:
                        RHEA_NO_COPY_NO_ASSIGN(AllocatorTrackingPolicy_none);
    };



    /***********************************************************************
    * AllocatorTrackingPolicy_simple
    *
    * conta il numero di alloazione/deallocazioni e tira una
    * eccezione se il conteggio non torna a zero dopo la rhea::deinit()
    ***********************************************************************/
    class AllocatorTrackingPolicy_simple
    {
    public:
                        AllocatorTrackingPolicy_simple () :
                            nalloc(0), curMemAlloc(0), maxMemalloc(0)
                        {
                        }

                        ~AllocatorTrackingPolicy_simple()													{}

        bool			anyMemLeaks()                                                                       { return (nalloc>0); }

        void			onAlloc (size_t size)
                        {
                            ++nalloc;
                            curMemAlloc += size;
                            if (curMemAlloc > maxMemalloc)
                                maxMemalloc = curMemAlloc;
                        }

        void			onDealloc (size_t size)
                        {
                            assert (nalloc>0);
                            --nalloc;
                            assert (curMemAlloc >= size);
                            curMemAlloc -= size;
                        }

    public:
        u32             nalloc;
        size_t          curMemAlloc;
        size_t          maxMemalloc;

    private:
                        RHEA_NO_COPY_NO_ASSIGN(AllocatorTrackingPolicy_simple);

    };
}
#endif // _rheaAllocatorTrackingPolicy_h_

