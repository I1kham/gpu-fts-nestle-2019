#ifndef _SocketBridgeTaskFactory_h_
#define _SocketBridgeTaskFactory_h_
#include "SocketBridgeTask.h"
#include "../rheaCommonLib/rheaFastArray.h"



typedef  socketbridge::Task* (*SocketBridgeTaskSpawnFn) (rhea::Allocator *allocator);

namespace socketbridge
{
	/***************************************************************
	 *
	 *	TaskFactory
	 *
	 */
	class TaskFactory
	{
	public:
						TaskFactory();
						~TaskFactory();


						template<class TTask>
		void			add (const char *taskName)
						{
							u32 i = list.getNElem();
							list[i].spawnFn = TTask::spawn;
							list[i].name = rhea::string::alloc(localAllocator, taskName);
						}

		Task*			spawn (rhea::Allocator *allocator, const char *taskName) const;
		TaskStatus*		spawnAndRunTask(rhea::Allocator *allocator, const char *taskName, const char *params) const;

	private:
		struct sRecord
		{
			SocketBridgeTaskSpawnFn	spawnFn;
			char					*name;
		};

	private:
		rhea::Allocator				*localAllocator;
		rhea::FastArray<sRecord>	list;
	};



} // namespace socketbridge

#endif // _SocketBridgeTaskFactory_h_
