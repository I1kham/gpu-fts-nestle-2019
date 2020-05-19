#include "rheaLogTargetFile.h"
#include "rheaString.h"
#include "rhea.h"


using namespace rhea;


//*********************************************
LogTargetFile::~LogTargetFile()
{
    rhea::getSysHeapAllocator()->dealloc(filename);
}

//*********************************************
bool LogTargetFile::init (const char *filenameIN, bool bDeleteFileOnStartup)
{
    filename = string::utf8::allocStr (rhea::getSysHeapAllocator(), filenameIN);
    if (bDeleteFileOnStartup)
        rhea::fs::fileDelete (filename);

    FILE *f = fs::fileOpenForWriteText(filename);
    if (NULL != f)
    {
        fclose(f);
        return true;
    }
    return false;
}




//*********************************************
void LogTargetFile::doLog	(u32 channel UNUSED_PARAM, const char *msg)
{
    FILE *f = fs::fileOpenForAppendText(filename);
    if (NULL != f)
    {
        fprintf (f, "%s", msg);
        fclose(f);
    }
}
