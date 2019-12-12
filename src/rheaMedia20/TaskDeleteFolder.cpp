#include "TaskDeleteFolder.h"




//*********************************************************************
void TaskDeleteFolder::run (socketbridge::TaskStatus *status, const char *params)
{
	if (NULL == params)
		return;
	if (params[0] == 0)
		return;

	char path[512];
	rhea::fs::sanitizePath(params, path, sizeof(path));
	rhea::fs::deleteAllFileInFolderRecursively (path, true);


	status->setMessage("OK");
}