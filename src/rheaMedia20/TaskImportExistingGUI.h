#ifndef _TaskImportExistingGUI_h_
#define _TaskImportExistingGUI_h_
#include "../SocketBridge/SocketBridgeTask.h"


/***************************************************************
 *
 * TaskImportExistingGUI
 *
 *	params: templateName § templateVer § templateSrcPath § userGUISrcPath § dstPath
 *
 *  crea [dstPath]
 *	copia [templateSrcPath] in [dstPath]
 *	in base a [templateName], effettua altre operazioni
 */
class TaskImportExistingGUI : public socketbridge::Task
{
public:
									TaskImportExistingGUI()				{ };
	void							run (socketbridge::TaskStatus *status, const char *params);

	static socketbridge::Task*		spawn (rhea::Allocator *allocator) { return RHEANEW(allocator, TaskImportExistingGUI); }

private:
	u32 ct;
};


#endif // _TaskImportExistingGUI_h_
