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
	static void						priv_updateLanguageName(const char *dbPath);

	
public:
									TaskImportExistingGUI()				{ };
	void							run (socketbridge::TaskStatus *status, const char *params);

	static socketbridge::Task*		spawn (rhea::Allocator *allocator) { return RHEANEW(allocator, TaskImportExistingGUI); }



private:
	bool							priv_nestle20_template001_importVersion0(const char *userGUISrcPath, const char *dstPath);

private:
	u32 ct;
};


#endif // _TaskImportExistingGUI_h_
