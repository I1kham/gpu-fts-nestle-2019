#include "../CPUBridge/CPUBridge.h"
#include "../CPUBridge/CPUChannelCom.h"
#include "../CPUBridge/CPUChannelFakeCPU.h"
#include "../SocketBridge/SocketBridge.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"
#include "../CPUBridge/EVADTSParser.h"
#include "../rheaExternalSerialAPI/ESAPI.h"

//nome della porta seriale
#ifdef PLATFORM_UBUNTU_DESKTOP
    #define CPU_COMPORT     "/dev/ttyUSB0"
    #define ESAPI_COMPORT	"/dev/ttyUSB1"
#elif PLATFORM_YOCTO_EMBEDDED
    #define CPU_COMPORT     "/dev/ttymxc3"
    #define ESAPI_COMPORT   "/dev/ttymxc2"
#else
	#define CPU_COMPORT		"COM5"
	#define ESAPI_COMPORT	"COM7"
#endif




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
    //cpubridge::CPUChannelFakeCPU *chToCPU = new cpubridge::CPUChannelFakeCPU(); bool b = chToCPU->open (logger);
	
	//apro un canale con la CPU fisica
	cpubridge::CPUChannelCom *chToCPU = new cpubridge::CPUChannelCom();    bool b = chToCPU->open("COM8", logger);
#endif

    if (!b)
        return false;

    //creo il thread di CPUBridge
    rhea::HThread hCPUThread;
	HThreadMsgW hCPUServiceChannelW;
    if (!cpubridge::startServer(chToCPU, logger, &hCPUThread, &hCPUServiceChannelW))
		return false;


	//starto socketBridge che a sua volta si iscriverà a CPUBridge
	rhea::HThread hSocketBridgeThread;
	socketbridge::startServer(logger, hCPUServiceChannelW, false, true, &hSocketBridgeThread);


	//starto il thread ESAPI
	rhea::HThread hESAPIThread;	
	esapi::startThread (ESAPI_COMPORT, hCPUServiceChannelW, logger, &hESAPIThread);


	//attendo che il thread CPU termini
	rhea::thread::waitEnd (hCPUThread);

	return true;
}




//*****************************************************
int main()
{
#ifdef WIN32
	HINSTANCE hInst = NULL;
    rhea::init("rheaSMU", &hInst);
#else
	rhea::init("rheaSMU", NULL);
#endif

    //mac address
    {
        char mac[16];
        rhea::netaddr::getMACAddress(mac, sizeof(mac));
        printf ("MAC ADDRESS: %s\n", mac);
    }
	startCPUBridge();

	//startSyandAloneSocketBridge();

    rhea::deinit();
    return 0;
}
