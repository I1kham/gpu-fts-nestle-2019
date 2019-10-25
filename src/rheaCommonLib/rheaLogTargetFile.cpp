#include "rheaLogTargetFile.h"
#include "rheaString.h"
#include "rheaMemory.h"
#include "OS/OS.h"


using namespace rhea;


//*********************************************
LogTargetFile::~LogTargetFile()
{
    memory_getDefaultAllocator()->dealloc(filename);
}

//*********************************************
bool LogTargetFile::init (const char *filenameIN, bool bDeleteFileOnStartup)
{
    filename = string::alloc (memory_getDefaultAllocator(), filenameIN);
    if (bDeleteFileOnStartup)
        remove(filename);

    FILE *f = fopen(filename, "wt");
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
    FILE *f = fopen(filename, "at");
    if (NULL != f)
    {
        fprintf (f, "%s", msg);
        fclose(f);
    }
}
