#include "rheaMemory.h"
#include "rheaAllocatorSimple.h"
#include "rheaMemoryTracker.h"


rhea::AllocatorSimpleWithMemTrack *defaultAllocator = NULL;
size_t  sizeOfAPointer = 0;
rhea::MemoryTracker *memTracker = NULL;


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

	if (memTracker)
	{
		delete memTracker;
		memTracker = NULL;
	}
}

//**************************************
rhea::MemoryTracker* rhea::internal_getMemoryTracker()
{
	if (NULL == memTracker)
		memTracker = new rhea::MemoryTracker();
	return memTracker;
}


//**************************************
size_t rhea::memory_getSizeOfAPointer()
{
    return sizeOfAPointer;
}


//**************************************
rhea::Allocator* rhea::memory_getSysHeapAllocator()
{
    return defaultAllocator;
}

//**************************************
rhea::Allocator* rhea::memory_getScrapAllocator2()
{
	return defaultAllocator;
}

