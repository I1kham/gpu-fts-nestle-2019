#include "SocketBridgeTaskFactory.h"


using namespace socketbridge;

//*********************************************
TaskFactory::TaskFactory()
{
	localAllocator = rhea::getScrapAllocator();
	list.setup(localAllocator, 64);
}


//*********************************************
TaskFactory::~TaskFactory()
{
	u32 n = list.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		RHEAFREE(localAllocator, list(i).name);
	}
	list.unsetup();
}


//*********************************************
Task* TaskFactory::spawn(rhea::Allocator *allocator, const char *taskName) const
{
	u32 n = list.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		if (strcasecmp(taskName, list(i).name) == 0)
			return list(i).spawnFn(allocator);
	}
	return NULL;
}

//*********************************************
TaskStatus*	TaskFactory::spawnAndRunTask (rhea::Allocator *allocator, const char *taskName, const u8 *params) const
{
	Task *task = spawn (allocator, taskName);
	if (NULL == task)
		return NULL;

	TaskStatus*	status = RHEANEW(allocator, TaskStatus)();
	status->_localAllocator = allocator;
	status->_task = task;
	if (NULL != params)
		status->params = rhea::string::utf8::allocStr(allocator, params);

	rhea::HThread hThread;
	eThreadError err = rhea::thread::create(&hThread, socketbridge::SocketBridgeTaskThreadFn, status, 1024);
	if (err != eThreadError::none)
	{
		RHEADELETE(allocator, status->_task);
		RHEADELETE(allocator, status);
		return NULL;
	}

	return status;
}
