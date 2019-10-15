#include "header.h"
#include "mainwindow.h"
#include <QApplication>
#include "../CPUBridge/CPUBridge.h"
#include "../CPUBridge/CPUChannelCom.h"
#include "../CPUBridge/CPUChannelFakeCPU.h"
#include "../SocketBridge/SocketBridge.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"


MainWindow *myMainWindow = NULL;


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
    rhea::ISimpleLogger *logger = &loggerSTD;
#else
    rhea::NullLogger loggerNULL;
    rhea::ISimpleLogger *logger = &loggerNULL;
#endif


#ifdef PLATFORM_YOCTO_EMBEDDED
    //apro un canale di comunicazione con la CPU fisica
    cpubridge::CPUChannelCom chToCPU;
    bool b = chToCPU.open(CPU_COMPORT, &logger);
#else
    //apro un canale di comunicazione con una finta CPU
    cpubridge::CPUChannelFakeCPU chToCPU;
    bool b = chToCPU.open (logger);
#endif

    if (!b)
        return false;

    //creo il thread di CPUBridge
    rhea::HThread hCPUThread;
    HThreadMsgW hCPUServiceChannelW;

    if (!cpubridge::startServer(&chToCPU, logger, &hCPUThread, &hCPUServiceChannelW))
        return false;

    //starto socketBridge che a sua volta siiscriverà a CPUBridge
    rhea::HThread hSocketBridgeThread;
    startSocketBridge(hCPUServiceChannelW, logger, &hSocketBridgeThread);


    //attendo che il thread CPU termini
    //rhea::thread::waitEnd (hCPUThread);

    return true;
}




//****************************************************
unsigned char _button_keyNum_tx=0;
unsigned char getButtonKeyNum ()    { return _button_keyNum_tx; }
void setButtonKeyNum (unsigned char i)
{
    if (_button_keyNum_tx != i)
    {
        DEBUG_MSG ("Button keynum=%d", (int)i);
        _button_keyNum_tx=i;
    }
}





//****************************************************
int main(int argc, char *argv[])
{
#ifdef WIN32
    HINSTANCE hInst = NULL;
    rhea::init("rheaSMU", &hInst);
#else
    rhea::init("rheaSMU", NULL);
#endif

    startCPUBridge();

    QApplication app(argc, argv);
    hideMouse();

    utils::gatherFolderInfo (qApp->applicationDirPath());

    myMainWindow = new MainWindow ();
    myMainWindow->show();

    int ret = app.exec();

    rhea::deinit();
    return ret;
}
