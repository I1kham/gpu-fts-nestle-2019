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
