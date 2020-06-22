#include "raspiCore.h"

//*****************************************************
bool startSocket2280 (rhea::ISimpleLogger *logger)
{
	raspi::Core core;
	core.useLogger (logger);
	if (!core.open ("COM3"))
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



    rhea::deinit();
    return 0;
}
