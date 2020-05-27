#include "rasPIMITM.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"
#include "../SocketBridge/SocketBridge.h"

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
#ifdef LINUX
    const char serialPortToCPU[] = {"/dev/ttyUSB0"};
    const char serialPortToGPU[] = {"/dev/ttyUSB1"};
#else
    const char serialPortToCPU[] = {"COM4"};\
    const char serialPortToGPU[] = {"COM3"};
#endif
    rhea::HThread hMITMThread;
	HThreadMsgW msgQW_toMITM;
    if (!rasPI::MITM::start (logger, serialPortToGPU, serialPortToCPU, &hMITMThread, &msgQW_toMITM))
		return false;


	/*starto socketBridge che a sua volta si iscriverà a MITM
	rhea::HThread hSocketBridgeThread;
	socketbridge::startServer(logger, msgQW_toMITM, false, &hSocketBridgeThread);
	*/


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
