#ifdef LINUX
#include "linuxOS.h"
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <time.h>


//*******************************************************************
eThreadError platform::createThread (OSThread &out_handle, OSThreadFunction threadFunction, size_t stackSizeInKb, void *userParam)
{
    pthread_attr_t  attr;
    int rc = pthread_attr_init(&attr);
    if (rc != 0)
        return eThreadError::unknown;

    size_t stackSizeInBytes = stackSizeInKb*1024;
    if (stackSizeInBytes < PTHREAD_STACK_MIN)
        stackSizeInBytes = PTHREAD_STACK_MIN;
    rc = pthread_attr_setstacksize(&attr, stackSizeInBytes);
    if (rc != 0)
        return eThreadError::invalidStackSize;

    rc = pthread_create(&out_handle, &attr, threadFunction, userParam);
    if (rc == 0)
        return eThreadError::none;

    switch (rc)
    {
        case EAGAIN:    return eThreadError::tooMany;
        default:        return eThreadError::unknown;
    }
}

//*******************************************************************
void platform::killThread (OSThread &handle UNUSED_PARAM)
{
    //pthread_exit(handle);
}

//*******************************************************************
void platform::waitThreadEnd(OSThread &handle)
{
    void *ret = 0;
    pthread_join(handle, &ret);
}





#endif
