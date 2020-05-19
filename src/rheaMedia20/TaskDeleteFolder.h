#ifndef _TaskDeleteFolder_h_
#define _TaskDeleteFolder_h_
#include "../SocketBridge/SocketBridgeTask.h"


/***************************************************************
 *
 * TaskDeleteFolder
 *
 *	params: path
 */
class TaskDeleteFolder : public socketbridge::Task
{
public:
									TaskDeleteFolder()				{ };
	void							run (socketbridge::TaskStatus *status, const u8 *params);

	static socketbridge::Task*		spawn (rhea::Allocator *allocator) { return RHEANEW(allocator, TaskDeleteFolder); }

private:
	u32 ct;
};


#endif // _TaskDeleteFolder_h_
