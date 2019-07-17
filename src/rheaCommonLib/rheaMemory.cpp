#include "rheaMemory.h"
#include "rheaAllocatorSimple.h"


rhea::AllocatorSimpleWithMemTrack *defaultAllocator = NULL;
size_t  sizeOfAPointer = 0;


//**************************************
bool rhea::internal_memory_init()
{
    sizeOfAPointer = sizeof(void*);
    if (sizeOfAPointer <= 4)
        sizeOfAPointer = 4;
    else if (sizeOfAPointer <= 8)
        sizeOfAPointer = 8;


    defaultAllocator = new rhea::AllocatorSimpleWithMemTrack("defaultA");
    return true;
}

//**************************************
void rhea::internal_memory_deinit()
{
    if (defaultAllocator)
        delete defaultAllocator;
    defaultAllocator=NULL;
}


//**************************************
size_t rhea::memory_getSizeOfAPointer()
{
    return sizeOfAPointer;
}


//**************************************
rhea::Allocator* rhea::memory_getDefaultAllocator()
{
    return defaultAllocator;
}



