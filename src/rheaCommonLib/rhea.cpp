#include <limits.h>
#include "rhea.h"
#include "rheaMemory.h"
#include "rheaThread.h"
#include "rheaLogTargetFile.h"

namespace rhea
{
    Logger   *logger = NULL;
}

struct sRheaGlobals
{
    rhea::LogTargetFile     *fileLogger;
};
sRheaGlobals	rheaGlobals;


//***************************************************
bool rhea::init (void *platformSpecificInitData)
{
    //init random generator
    srand((unsigned) time(NULL));

	//init OS Stuff
	if (!internal_OSInit(platformSpecificInitData))
		return false;

    //init memory
    rhea::internal_memory_init();


    //init logger
    logger = RHEANEW(memory_getDefaultAllocator(), Logger)(true);

    //aggiungo un file logger
    {
        char logFileName[1024];
        sprintf_s (logFileName, sizeof(logFileName), "%s/log.txt", rhea::getPhysicalPathToAppFolder());

        rheaGlobals.fileLogger = RHEANEW(memory_getDefaultAllocator(), rhea::LogTargetFile)();
        if (!rheaGlobals.fileLogger->init (logFileName, true))
            return false;
        logger->addTarget(rheaGlobals.fileLogger);
    }

    logger->log ("**********************************************************************************", false, false) << Logger::EOL;
    logger->log ("",true,false) << Logger::EOL;
    logger->log ("**********************************************************************************", false, false) << Logger::EOL;


    //init thread
    thread::internal_init();



    //fine
    logger->log ("rhea::init  DONE!") << Logger::EOL;
    return true;
}

//***************************************************
void rhea::deinit()
{
    logger->log ("", false, false) << Logger::EOL;
    logger->log ("Kernel::Deinit  STARTED") << Logger::EOL;


    logger->log ("thread::Deinit  STARTED") << Logger::EOL;
    thread::internal_deinit();

    logger->log ("mem::Deinit  STARTED") << Logger::EOL;
    RHEADELETE(memory_getDefaultAllocator(), logger);
    RHEADELETE(memory_getDefaultAllocator(), rheaGlobals.fileLogger);
    rhea::internal_memory_deinit();

	internal_OSDeinit();
}
