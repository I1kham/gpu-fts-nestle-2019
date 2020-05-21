#include "socketListener.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"


//*****************************************************
bool startSokListener()
{
#ifdef _DEBUG
	rhea::StdoutLogger loggerSTD; 
	rhea::ISimpleLogger *logger = &loggerSTD;
#else
	rhea::NullLogger loggerNULL;
	rhea::ISimpleLogger *logger = &loggerNULL;
#endif


    //creo il thread
    rhea::HThread hSokListenerThread;
    if (!rasPI::socketListener::start(logger, &hSokListenerThread))
		return false;


	//attendo che il thread termini
	rhea::thread::waitEnd (hSokListenerThread);

	return true;
}

//*****************************************************
int main()
{
#ifdef WIN32
	HINSTANCE hInst = NULL;
    rhea::init("rheaRasPI", &hInst);
#else
	rhea::init("rheaRasPI", NULL);
#endif

	startSokListener();

    rhea::deinit();
    return 0;
}
