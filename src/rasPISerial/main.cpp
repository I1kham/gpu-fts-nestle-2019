#include "raspiCore.h"
#include "../rheaCommonLib/SimpleLogger/FileLogger.h"

#define FW_VERSION "1.0"


//*****************************************************
bool startESAPI ()
{
#ifdef _DEBUG
	rhea::StdoutLogger loggerSTD; 
    rhea::ISimpleLogger *logger = &loggerSTD;
#else
    //rhea::NullLogger loggerNULL;
    //rhea::ISimpleLogger *logger = &loggerNULL;
    rhea::FileLogger loggerFile((const u8*)"/home/pi/rhea/gpu-fts-nestle-2019/bin/output.log");
    rhea::ISimpleLogger *logger = &loggerFile;
#endif

#ifdef PLATFORM_RASPI
    const char SERIAL_PORT[] = {"/dev/ttyAMA0"};
#else
    const char SERIAL_PORT[] = {"COM5"};
#endif

    logger->log ("FW version: " FW_VERSION "\n");

	raspi::Core core;
	core.useLogger (logger);
    if (!core.open (SERIAL_PORT))
		return false;
	
	core.run();
	return true;
}

#include "../rheaCommonLib/compress/rheaCompress.h"

//*****************************************************
int main()
{
#ifdef WIN32
	HINSTANCE hInst = NULL;
    rhea::init("rheaRasPIESAPI", &hInst);
#else
	rhea::init("rheaRasPIESAPI", NULL);
#endif


	startESAPI();

    rhea::deinit();
    return 0;
}
