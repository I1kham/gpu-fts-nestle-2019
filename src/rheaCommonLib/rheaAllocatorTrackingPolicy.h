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
        void			onDealloc ()                                                    { }

    private:
                        RHEA_NO_COPY_NO_ASSIGN(AllocatorTrackingPolicy_none);
    };



    /***********************************************************************
    * AllocatorTrackingPolicy_simple
    *
    *
    ***********************************************************************/
    class AllocatorTrackingPolicy_simple
    {
    public:
                        AllocatorTrackingPolicy_simple () :
                            nalloc(0), maxMemalloc(0)
                        {
                        }

                        ~AllocatorTrackingPolicy_simple()													{}

        bool			anyMemLeaks()                                                                       { return (nalloc>0); }

        void			onAlloc (size_t size)
                        {
                            ++nalloc;
                            maxMemalloc += size;
                        }

        void			onDealloc ()
                        {
                            assert (nalloc>0);
                            --nalloc;
                        }

    public:
        u32             nalloc;
        u32             maxMemalloc;

    private:
                        RHEA_NO_COPY_NO_ASSIGN(AllocatorTrackingPolicy_simple);

    };
}
#endif // _rheaAllocatorTrackingPolicy_h_

