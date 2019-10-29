#ifndef _utils_h_
#define _utils_h_
#include "../rheaCommonLib/rheaDataTypes.h"
#include <qstring.h>
#include <qfont.h>


namespace utils
{
    void                hideMouse();
    void                getRightFontForLanguage (QFont &out, int pointSize, const char *iso2LettersLanguageCode);

    double              updateCPUStats(unsigned long timeSinceLastCallMSec);
    bool                copyRecursively (const QString &srcFilePath, const QString &tgtFilePath);

    void                waitAndProcessEvent (u32 msecToWait);
}

#endif // _utils_h_
