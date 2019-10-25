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



#ifdef PLATFORM_YOCTO_EMBEDDED
    //apro un canale di comunicazione con la CPU fisica
    cpubridge::CPUChannelCom *chToCPU = new cpubridge::CPUChannelCom();
    bool b = chToCPU->open("/dev/ttymxc3", logger);
#else
    //apro un canale di comunicazione con una finta CPU
    cpubridge::CPUChannelFakeCPU *chToCPU = new cpubridge::CPUChannelFakeCPU(); bool b = chToCPU->open (logger);

    //apro un canale con la CPU fisica
    //cpubridge::CPUChannelCom *chToCPU = new cpubridge::CPUChannelCom();    bool b = chToCPU->open(CPU_COMPORT, logger);

#endif

    if (!b)
        return false;



	//creo il thread di CPUBridge
	rhea::HThread hCPUThread;
	HThreadMsgW hCPUServiceChannelW;

    if (!cpubridge::startServer(chToCPU, logger, &hCPUThread, &hCPUServiceChannelW))
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
