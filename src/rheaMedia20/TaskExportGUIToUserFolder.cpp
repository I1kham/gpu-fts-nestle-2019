#include "TaskExportGUIToUserFolder.h"




//*********************************************************************
void TaskExportGUIToUserFolder::run(socketbridge::TaskStatus *status, const char *params)
{
	status->setMessage("TaskExportGUIToUserFolder::A");
	rhea::thread::sleepMSec(2000);


	status->setMessage("TaskExportGUIToUserFolder::B");
}