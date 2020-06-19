#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"


//*****************************************************
bool startSocket2280 (rhea::ISimpleLogger *logger)
{
    OSSocket sok;
    rhea::socket::init (&sok);

    logger->log ("opening socket on 2280...");
    eSocketError err = rhea::socket::openAsTCPServer(&sok, 2280);
    if (err != eSocketError_none)
    {
        logger->log ("ERR code[%d]\n", err);
        logger->log("\n");
        return false;
    }
}



//*****************************************************
int main()
{
#ifdef WIN32
	HINSTANCE hInst = NULL;
    rhea::init("rheaRasPISerial", &hInst);
#else
	rhea::init("rheaRasPISerial", NULL);
#endif



    rhea::deinit();
    return 0;
}
