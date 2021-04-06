#ifdef WIN32
#include "winOS.h"

struct sThreadBootstrap
{
	OSThreadFunction	fn;
	void				*userData;
};
sThreadBootstrap	threadBootstrap[32];
u32					nextThreadBootstrap = 0;

//*********************************************
DWORD WINAPI Win32InternalThreadProc (void *lpParameter)
{
	sThreadBootstrap *init = (sThreadBootstrap*)lpParameter;
	OSThreadFunction fn = init->fn;
	void *userData = init->userData;

	void *ret = fn(userData);
	return 0;
}


//*******************************************************************
eThreadError platform::createThread (OSThread &out_handle, OSThreadFunction threadFunction, size_t stackSizeInKb, void *userParam)
{
	u32 flag = 0;
	//if (bStartSuspended)	flag |= CREATE_SUSPENDED;


	sThreadBootstrap *init = &threadBootstrap[nextThreadBootstrap++];
	if (nextThreadBootstrap == 32)
		nextThreadBootstrap = 0;

	init->fn = threadFunction;
	init->userData = userParam;
	out_handle = ::CreateThread(NULL, 0, Win32InternalThreadProc, (void*)init, flag, NULL);

	if (NULL == INVALID_HANDLE_VALUE)
		return eThreadError::unknown;

	return eThreadError::none;
}

//*******************************************************************
void platform::killThread (OSThread &handle)
{
	if (NULL == handle)
		return;
	::CloseHandle(handle);
	handle = NULL;
}

//*******************************************************************
void platform::waitThreadEnd (OSThread &handle)
{
	WaitForSingleObject (handle, INFINITE);
}
#endif //WIN32
