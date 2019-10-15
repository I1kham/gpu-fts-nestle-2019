#ifndef _utils_h_
#define _utils_h_
#include "../rheaCommonLib/rheaDataTypes.h"
#include <qstring.h>
#include <qfont.h>


namespace utils
{
    void                hideMouse();
    void                getRightFontForLanguage (QFont &out, int pointSize, const char *iso2LettersLanguageCode);

    bool                isUsbPresent();

    void                gatherFolderInfo (const QString &Folder_appGpu);
    const QString&      getFolder_VMCSettings();
    const QString&      getFolder_CPU();
    const QString&      getFolder_GUI();
    const QString&      getFolder_Manual();
    const QString&      getFolder_Lang();

    const QString&      getFolder_Usb_VMCSettings();
    const QString&      getFolder_Usb_CPU();
    const QString&      getFolder_Usb_GUI();
    const QString&      getFolder_Usb_Manual();
    const QString&      getFolder_Usb_Audit();
    const QString&      getFolder_Usb_Lang();

    double              updateCPUStats();
}

#endif // _utils_h_
