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
bool startCPUBridge(HThreadMsgW *hCPUServiceChannelW)
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


    if (!cpubridge::startServer(&chToCPU, logger, &hCPUThread, hCPUServiceChannelW))
        return false;

    //starto socketBridge che a sua volta siiscriver√  a CPUBridge
    rhea::HThread hSocketBridgeThread;
    startSocketBridge(*hCPUServiceChannelW, logger, &hSocketBridgeThread);


    //attendo che il thread CPU termini
    //rhea::thread::waitEnd (hCPUThread);

    return true;
}



//****************************************************
bool subscribeToCPU (const HThreadMsgW hCPUServiceChannelW, cpubridge::sSubscriber *out_subscriber)
{
    bool ret = false;

    //creo una msgQ temporanea per ricevere da CPUBridge la risposta alla mia richiesta di iscrizione
    HThreadMsgR hMsgQR;
    HThreadMsgW hMsgQW;
    rhea::thread::createMsgQ (&hMsgQR, &hMsgQW);

    //invio la richiesta
    cpubridge::subscribe (hCPUServiceChannelW, hMsgQW);

    //attendo risposta
    u64 timeToExitMSec = rhea::getTimeNowMSec() + 2000;
    do
    {
        rhea::thread::sleepMSec(50);

        rhea::thread::sMsg msg;
        if (rhea::thread::popMsg(hMsgQR, &msg))
        {
            //ok, ci siamo
            if (msg.what == CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER)
            {
                u8 cpuBridgeVersion = 0;
                cpubridge::translate_SUBSCRIPTION_ANSWER (msg, out_subscriber, &cpuBridgeVersion);
                rhea::thread::deleteMsg(msg);
                ret = true;
                break;
            }

            rhea::thread::deleteMsg(msg);
        }
    } while (rhea::getTimeNowMSec() < timeToExitMSec);

    //delete della msgQ
    rhea::thread::deleteMsgQ (hMsgQR, hMsgQW);


    return ret;
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

    //Avvio della SMU
    HThreadMsgW hCPUServiceChannelW;
    startCPUBridge(&hCPUServiceChannelW);


    //Mi iscrivo alla CPU per ricevere direttamente le notifiche che questa manda al cambiare del suo stato
    cpubridge::sSubscriber subscriber;
    subscribeToCPU (hCPUServiceChannelW, &subscriber);



    QApplication app(argc, argv);
    utils::hideMouse();

    utils::gatherFolderInfo (qApp->applicationDirPath());

    myMainWindow = new MainWindow(subscriber);
    myMainWindow->show();

    int ret = app.exec();

    rhea::deinit();
    return ret;
}
