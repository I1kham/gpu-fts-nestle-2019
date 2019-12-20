#include "SocketBridgeTask.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

using namespace socketbridge;

u32 TaskStatus::nextUID = 0x01;

//***********************************************************
TaskStatus::TaskStatus ()
{
	rhea::criticalsection::init(&cs);
	status = eStatus_pending;
	msg[0] = 0;
	timeFinishedMSec = 0;
	params = NULL;
	_task = NULL;
	_localAllocator = NULL;

	rhea::criticalsection::enter(cs);
	{
		uid = TaskStatus::nextUID++;
	}
	rhea::criticalsection::leave(cs);
}

//***********************************************************
TaskStatus::~TaskStatus()
{
	rhea::criticalsection::close(cs);
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
	rhea::criticalsection::enter(cs);
	{
		priv_doSetStatusNoCS(s);
	}
	rhea::criticalsection::leave(cs);
}

//***********************************************************
void TaskStatus::setMessage (const char *format, ...)
{
	va_list argptr;
	va_start (argptr, format);

	rhea::criticalsection::enter(cs);
	{
		vsnprintf(msg, sizeof(msg), format, argptr);
	}
	rhea::criticalsection::leave(cs);

	va_end(argptr);
}

//***********************************************************
void TaskStatus::setStatusAndMessage(eStatus s, const char *format, ...)
{
	va_list argptr;
	va_start(argptr, format);

	rhea::criticalsection::enter(cs);
	{
		vsnprintf(msg, sizeof(msg), format, argptr);
		priv_doSetStatusNoCS(s);
	}
	rhea::criticalsection::leave(cs);

	va_end(argptr);
}

//***********************************************************
void TaskStatus::getStatusAndMesssage (eStatus *out_status, char *out_msg, u32 sizeofmsg)
{
	rhea::criticalsection::enter(cs);
	{
		*out_status = status;
		strcpy_s(out_msg, sizeofmsg, msg);
	}
	rhea::criticalsection::leave(cs);
}

