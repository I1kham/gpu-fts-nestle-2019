#ifndef _TaskCopyGUIToLocalTempFolder_h_
#define _TaskCopyGUIToLocalTempFolder_h_
#include "../SocketBridge/SocketBridgeTask.h"


/***************************************************************
 *
 */
class TaskCopyGUIToLocalTempFolder : public socketbridge::Task
{
public:
						TaskCopyGUIToLocalTempFolder()				{ };
	void				run (socketbridge::TaskStatus *status, const char *params);

	static socketbridge::Task*		spawn (rhea::Allocator *allocator) { return RHEANEW(allocator, TaskCopyGUIToLocalTempFolder); }

private:
	u32 ct;
};


#endif // _TaskCopyGUIToLocalTempFolder_h_
