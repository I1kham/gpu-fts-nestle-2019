#ifndef _TaskExportMobileTPGUIToUserFolder_h_
#define _TaskExportMobileTPGUIToUserFolder_h_
#include "../SocketBridge/SocketBridgeTask.h"


/***************************************************************
 *
 * TaskExportMobileTPGUIToUserFolder
 *
 *	params: srcTempFolderName § dstPath
 *
 *  copia la gui da [srcTempFolderName] in [dstPath] e la zippa in un unico file di nome mobile.rheaRasGuiTP
 */
class TaskExportMobileTPGUIToUserFolder : public socketbridge::Task
{
public:
									TaskExportMobileTPGUIToUserFolder()				{ };
	void							run (socketbridge::TaskStatus *status, const u8 *params);

	static socketbridge::Task*		spawn (rhea::Allocator *allocator) { return RHEANEW(allocator, TaskExportMobileTPGUIToUserFolder); }

private:
	u32 ct;
};


#endif // _TaskExportMobileTPGUIToUserFolder_h_
