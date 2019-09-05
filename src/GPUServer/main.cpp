#include "../CPUBridge/CPUBridge.h"
#include "../CPUBridge/CPUChannelCom.h"
#include "../CPUBridge/CPUChannelFakeCPU.h"
#include "../SocketBridge/SocketBridge.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"



//*****************************************************
bool startSocketBridge (HThreadMsgW hCPUServiceChannelW, rhea::StdoutLogger *logger, rhea::HThread *out_hThread)
{
	return socketbridge::startServer(logger, hCPUServiceChannelW, out_hThread);
}



//*****************************************************
bool startCPUBridge()
{
	rhea::StdoutLogger logger;

	/*apro un canale di comunicazione con la CPU fisica
	cpubridge::CPUChannelCom chToCPU;
	bool b = chToCPU.open("COM5", &logger);
	if (!b)
		return false;
	*/

	cpubridge::CPUChannelFakeCPU chToCPU;
	bool b = chToCPU.open (&logger);
	if (!b)
		return false;

	//creo il thread di CPUBridge
	rhea::HThread hCPUThread;
	HThreadMsgW hCPUServiceChannelW;

	if (!cpubridge::startServer(&chToCPU, &logger, &hCPUThread, &hCPUServiceChannelW))
		return false;

	//starto socketBridge che a sua volta siiscriverà a CPUBridge
	rhea::HThread hSocketBridgeThread;
	startSocketBridge(hCPUServiceChannelW, &logger, &hSocketBridgeThread);


	//attendo che il thread CPU termini
	rhea::thread::waitEnd (hCPUThread);

	return true;
}


//*****************************************************
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


//*****************************************************
int main()
{
#ifdef WIN32
	HINSTANCE hInst = NULL;
    rhea::init(&hInst);
#else
	rhea::init(NULL);
#endif

	char out[64];
	rhea::string::format::currency(1, 2, '.', out, sizeof(out));
	rhea::string::format::currency(12, 2, '.', out, sizeof(out));
	rhea::string::format::currency(123, 2, '.', out, sizeof(out));
	rhea::string::format::currency(1234, 2, '.', out, sizeof(out));
	


	startCPUBridge();

	//startSyandAloneSocketBridge();

    rhea::deinit();
    return 0;
}

