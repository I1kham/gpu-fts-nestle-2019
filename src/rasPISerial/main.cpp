#include "raspiCore.h"

//*****************************************************
bool startESAPI ()
{
#ifdef _DEBUG
	rhea::StdoutLogger loggerSTD; 
	rhea::ISimpleLogger *logger = &loggerSTD;
#else
	rhea::NullLogger loggerNULL;
	rhea::ISimpleLogger *logger = &loggerNULL;
#endif

	raspi::Core core;
	core.useLogger (logger);
	if (!core.open ("COM5"))
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
