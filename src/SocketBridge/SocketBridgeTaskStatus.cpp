#include "SocketBridgeTask.h"

using namespace socketbridge;

u32 TaskStatus::nextUID = 0x01;

//***********************************************************
TaskStatus::TaskStatus ()
{
	OSCriticalSection_init(&cs);
	status = eStatus_pending;
	msg[0] = 0;
	timeFinishedMSec = 0;
	params = NULL;
	_task = NULL;
	_localAllocator = NULL;

	OSCriticalSection_enter(cs);
	{
		uid = TaskStatus::nextUID++;
	}
	OSCriticalSection_leave(cs);
}

//***********************************************************
TaskStatus::~TaskStatus()
{
	OSCriticalSection_close(cs);
	if (params)
		RHEAFREE(_localAllocator, params);
}

//***********************************************************
void TaskStatus::priv_doSetStatusNoCS (eStatus s)
{
	//la prima volta che vado in "finished", memorizzo l'ora 
	//Una volta entrato in "finished", non c'è modo di modificare ulteriormente lo stato
	if (s == eStatus_finished)
	{
		if (status != eStatus_finished)
		{
			timeFinishedMSec = rhea::getTimeNowMSec();
			status = eStatus_finished;
		}
		return;
	}

	status = s;
}

//***********************************************************
void TaskStatus::setStatus (eStatus s)
{
	OSCriticalSection_enter(cs);
	{
		priv_doSetStatusNoCS(s);
	}
	OSCriticalSection_leave(cs);
}

//***********************************************************
void TaskStatus::setMessage (const char *format, ...)
{
	va_list argptr;
	va_start (argptr, format);

	OSCriticalSection_enter(cs);
	{
		vsnprintf(msg, sizeof(msg), format, argptr);
	}
	OSCriticalSection_leave(cs);

	va_end(argptr);
}

//***********************************************************
void TaskStatus::setStatusAndMessage(eStatus s, const char *format, ...)
{
	va_list argptr;
	va_start(argptr, format);

	OSCriticalSection_enter(cs);
	{
		vsnprintf(msg, sizeof(msg), format, argptr);
		priv_doSetStatusNoCS(s);
	}
	OSCriticalSection_leave(cs);

	va_end(argptr);
}

//***********************************************************
void TaskStatus::getStatusAndMesssage (eStatus *out_status, char *out_msg, u32 sizeofmsg)
{
	OSCriticalSection_enter(cs);
	{
		*out_status = status;
		strcpy_s(out_msg, sizeofmsg, msg);
	}
	OSCriticalSection_leave(cs);
}

