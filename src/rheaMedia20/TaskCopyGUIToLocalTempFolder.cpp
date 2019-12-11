#include "TaskCopyGUIToLocalTempFolder.h"




//*********************************************************************
void TaskCopyGUIToLocalTempFolder::run(socketbridge::TaskStatus *status, const char *params)
{
	status->setMessage("TaskCopyGUIToLocalTempFolder::1 [%s]", params);
	rhea::thread::sleepMSec(2000);


	status->setMessage("TaskCopyGUIToLocalTempFolder::2");
}