#include "../CPUBridge/CPUBridge.h"
#include "../CPUBridge/CPUChannelCom.h"
#include "../CPUBridge/CPUChannelFakeCPU.h"
#include "../SocketBridge/SocketBridge.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"



//*****************************************************
bool startSocketBridge (HThreadMsgW hCPUServiceChannelW, rhea::ISimpleLogger *logger, rhea::HThread *out_hThread)
{
	return socketbridge::startServer(logger, hCPUServiceChannelW, out_hThread);
}



//*****************************************************
bool startCPUBridge()
{
#ifdef _DEBUG
	rhea::StdoutLogger loggerSTD; 
	//rhea::NullLogger loggerSTD;
	rhea::ISimpleLogger *logger = &loggerSTD;
#else
	rhea::NullLogger loggerNULL;
	rhea::ISimpleLogger *logger = &loggerNULL;
#endif

	

	//apro un canale di comunicazione con la CPU fisica
	//cpubridge::CPUChannelCom chToCPU; if (!chToCPU.open("COM5", logger)) return false;

	//apro un canale con la CPU fake
	cpubridge::CPUChannelFakeCPU chToCPU; if (!chToCPU.open(logger)) return false;
	

	//creo il thread di CPUBridge
	rhea::HThread hCPUThread;
	HThreadMsgW hCPUServiceChannelW;

	if (!cpubridge::startServer(&chToCPU, logger, &hCPUThread, &hCPUServiceChannelW))
		return false;

	//starto socketBridge che a sua volta siiscriverà a CPUBridge
	rhea::HThread hSocketBridgeThread;
	startSocketBridge(hCPUServiceChannelW, logger, &hSocketBridgeThread);


	//attendo che il thread CPU termini
	rhea::thread::waitEnd (hCPUThread);

	return true;
}


/*****************************************************
void startSyandAloneSocketBridge()
{
	rhea::StdoutLogger logger;

	HThreadMsgW hCPUServiceChannelW;
	hCPUServiceChannelW.setInvalid();

	rhea::HThread hSocketBridgeThread;
	if (!startSocketBridge(hCPUServiceChannelW, &logger, &hSocketBridgeThread))
		return;

	//attendo che il thread termini
	rhea::thread::waitEnd(hSocketBridgeThread);
}
*/

//*****************************************************
int main()
{
#ifdef WIN32
	HINSTANCE hInst = NULL;
    rhea::init("rheaSMU", &hInst);
#else
	rhea::init("rheaSMU", NULL);
#endif

	startCPUBridge();

	//startSyandAloneSocketBridge();

    rhea::deinit();
    return 0;
}

