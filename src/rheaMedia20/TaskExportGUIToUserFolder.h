#ifndef _TaskExportGUIToUserFolder_h_
#define _TaskExportGUIToUserFolder_h_
#include "../SocketBridge/SocketBridgeTask.h"


/***************************************************************
 *
 * TaskExportGUIToUserFolder
 *
 *	params: srcTempFolderName § dstPath
 *
 *  copia la gui da [srcTempFolderName] in [dstPath]
 */
class TaskExportGUIToUserFolder : public socketbridge::Task
{
public:
									TaskExportGUIToUserFolder()				{ };
	void							run (socketbridge::TaskStatus *status, const char *params);

	static socketbridge::Task*		spawn (rhea::Allocator *allocator) { return RHEANEW(allocator, TaskExportGUIToUserFolder); }

private:
	u32 ct;
};


#endif // _TaskExportGUIToUserFolder_h_
