#include "socketListener.h"
#include "rasPIMITM.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"


//*****************************************************
bool startMITM()
{
#ifdef _DEBUG
	rhea::StdoutLogger loggerSTD;
	rhea::ISimpleLogger *logger = &loggerSTD;
#else
	rhea::NullLogger loggerNULL;
	rhea::ISimpleLogger *logger = &loggerNULL;
#endif


	//creo il thread MITM
	const char serialPortToCPU[] = {"COM4"};
	const char serialPortToGPU[] = {"COM3"};
    rhea::HThread hMITMThread;
	HThreadMsgW msgQW_toMITM;
    if (!rasPI::MITM::start (logger, serialPortToGPU, serialPortToCPU, &hMITMThread, &msgQW_toMITM))
		return false;


    //creo il thread socketListener
    rhea::HThread hSokListenerThread;
    if (!rasPI::socketListener::start(logger, msgQW_toMITM, &hSokListenerThread))
		return false;



	//attendo che MITM termini
	rhea::thread::waitEnd (hMITMThread);

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

	startMITM();

    rhea::deinit();
    return 0;
}
