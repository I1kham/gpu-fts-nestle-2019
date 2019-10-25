#ifndef _rheaHandleArray_h_
#define _rheaHandleArray_h_
#include "rheaHandleBaseArray.h"
#include "rheaIntrusiveFreeList.h"

namespace rhea
{
    /*=================================================================================
     * HandleArray
     *
     * Versione NON thread safe di BaseHandlerArray.
     * Vedi GOSBaseHandleArray.h per info
     *=================================================================================*/
    template<typename T, typename THandle>
    class HandleArray : public BaseHandleArray<IntrusiveFreeList<T>, T, THandle>
    {
    public:
        HandleArray ()          { }
    };

} //namespace rhea

#endif // _rheaHandleArray_h_

