#ifndef _utils_h_
#define _utils_h_
#include "../rheaCommonLib/rheaDataTypes.h"
#include <qstring.h>
#include <qfont.h>


namespace utils
{
    void                DEBUG_MSG (const char* format, ...);
    void                hideMouse();
    void                getRightFontForLanguage (QFont &out, int pointSize, const char *iso2LettersLanguageCode);

    double              updateCPUStats(unsigned long timeSinceLastCallMSec);
    bool                copyRecursively (const QString &srcFilePath, const QString &tgtFilePath);
}

#endif // _utils_h_
