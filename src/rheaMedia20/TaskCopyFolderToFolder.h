#ifndef _TaskCopyFolderToFolder_h_
#define _TaskCopyFolderToFolder_h_
#include "../SocketBridge/SocketBridgeTask.h"


/***************************************************************
 *
 * TaskCopyFolderToFolder
 *
 *	params: srcPath§dstPath
 */
class TaskCopyFolderToFolder : public socketbridge::Task
{
public:
									TaskCopyFolderToFolder()				{ };
	void							run (socketbridge::TaskStatus *status, const u8 *params);

	static socketbridge::Task*		spawn (rhea::Allocator *allocator) { return RHEANEW(allocator, TaskCopyFolderToFolder); }

private:
	u32 ct;
};


#endif // _TaskCopyFolderToFolder_h_
