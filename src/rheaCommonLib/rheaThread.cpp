#include "rheaThread.h"
#include "OS/OS.h"

struct ThreadInfo
{
    OSThread                osThreadHandle;
    rhea::ThMainFunction    threadMainFn;
    void                    *userParam;
};

/************************************************************************
 *
 */
ThreadInfo* thHandleToThreadInfo (rhea::HThread thHandle)
{
    return (ThreadInfo*)thHandle;
}



/************************************************************************
 *
 */
void* threadFunctionWrapper (void *userParam)
{
    ThreadInfo *th = (ThreadInfo*)userParam;
    rhea::ThMainFunction mainFn = th->threadMainFn;

    //int retCode = (*mainFn)(th->userParam);
    (*mainFn)(th->userParam);

	OS_killThread(th->osThreadHandle);
	
	rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();
    RHEAFREE(allocator, th);

    
    return NULL;
}



/************************************************************************
 * thread_create
 *
 * Prova a creare un thread.
 * Se tutto ok, ritorna eThreadError_none, filla [out_thHandle] con l'handle del thread
 */
eThreadError rhea::thread::create (HThread *out_hThread, ThMainFunction threadFunction, void *userParam, u16 stackSizeInKb)
{
    *out_hThread = NULL;

    Allocator *allocator = memory_getDefaultAllocator();

    ThreadInfo *th = RHEAALLOCSTRUCT(allocator, ThreadInfo);
    memset (th, 0x00, sizeof(ThreadInfo));
    th->threadMainFn = threadFunction;
    th->userParam = userParam;

    eThreadError err = OS_createThread (th->osThreadHandle, threadFunctionWrapper, stackSizeInKb, th);

    if (err == eThreadError_none)
        *out_hThread = th;
    else
        allocator->dealloc(th);
    return err;
}



/************************************************************************
 * thread_waitEnd
 *
 */
void rhea::thread::waitEnd(const HThread hThread)
{
    ThreadInfo *th = thHandleToThreadInfo(hThread);
    if (NULL == th)
        return;
    OS_waitThreadEnd(th->osThreadHandle);
}

/************************************************************************
 * sleepMSec
 *
 */
void rhea::thread::sleepMSec (size_t msec)
{
    OS_sleepMSec(msec);
}
