#include "raspiCore.h"
#include "../rheaCommonLib/SimpleLogger/FileLogger.h"

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

	{
		rhea::CompressUtility cp;
		cp.begin((const u8*)"c:\\vuota\\pippo.rheazip", 8);
		cp.addFile ((const u8*)"E:\\rhea\\gpu-fts-nestle-2019\\bin\\RheaMedia20\\home\\img\\animationRound.gif", (const u8*)"zippedAnimationRound.gif");
		cp.end();
	}

	startESAPI();

    rhea::deinit();
    return 0;
}
