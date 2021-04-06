#include "SocketBridgeTask.h"


using namespace socketbridge;

//*********************************************
i16 socketbridge::SocketBridgeTaskThreadFn(void *userParam)
{
	TaskStatus *status = static_cast<TaskStatus*>(userParam);
	status->setStatus(TaskStatus::eStatus::running);
	status->_task->run(status, status->params);
	status->setStatus(TaskStatus::eStatus::finished);
	RHEADELETE(status->_localAllocator, status->_task);
	return 0;
}
