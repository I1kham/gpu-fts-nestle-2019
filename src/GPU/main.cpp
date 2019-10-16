#include "header.h"
#include "mainwindow.h"
#include <QApplication>
#include <QDir>
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


/****************************************************
 * Filla [glob] con i path dei vari folder utilizzati dalla GPU
 */
void setupFolderInformation (sGlobal *glob)
{
    char s[1024];
    rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();

    //local folders
    const char *baseLocalFolder = rhea::getPhysicalPathToWritableFolder();

    sprintf_s (s, sizeof(s), "%s/GUI", baseLocalFolder);
    glob->localFolder_GUI = rhea::string::alloc(allocator, s);


    //USB folders
#ifdef PLATFORM_YOCTO_EMBEDDED
    sprintf_s (s, sizeof(s), "/run/media/sda1/rhea");
#else
    sprintf_s (s, sizeof(s), "%s/simula-chiavetta-usb", baseLocalFolder);
#endif

    //vediamo se il folder della USB esiste
    if (QDir(s).exists())
    {
        glob->usbFolder = rhea::string::alloc(allocator, s);

        sprintf_s (s, sizeof(s), "%s/rheaData", glob->usbFolder);
        glob->usbFolder_VMCSettings = rhea::string::alloc(allocator, s);

        sprintf_s (s, sizeof(s), "%s/rheaFirmwareCPU01", glob->usbFolder);
        glob->usbFolder_CPUFW = rhea::string::alloc(allocator, s);

        sprintf_s (s, sizeof(s), "%s/rheaGUI", glob->usbFolder);
        glob->usbFolder_GUI = rhea::string::alloc(allocator, s);

        sprintf_s (s, sizeof(s), "%s/rheaDataAudit", glob->usbFolder);
        glob->usbFolder_Audit = rhea::string::alloc(allocator, s);

        sprintf_s (s, sizeof(s), "%s/lang", glob->usbFolder);
        glob->usbFolder_Lang = rhea::string::alloc(allocator, s);
    }
    else
    {
        glob->usbFolder = glob->usbFolder_VMCSettings = glob->usbFolder_CPUFW =
        glob->usbFolder_GUI = glob->usbFolder_Audit = glob->usbFolder_Lang = NULL;
    }
}



//****************************************************
int main(int argc, char *argv[])
{
    rhea::init("rheaGPU", NULL);

    //Avvio della SMU
    HThreadMsgW hCPUServiceChannelW;
    startCPUBridge(&hCPUServiceChannelW);


    //recupero informazioni sui vari folder
    sGlobal glob;
    setupFolderInformation(&glob);

    //Mi iscrivo alla CPU per ricevere direttamente le notifiche che questa manda al cambiare del suo stato
    subscribeToCPU (hCPUServiceChannelW, &glob.subscriber);


    //Avvio del main form
    QApplication app(argc, argv);
    utils::hideMouse();

    myMainWindow = new MainWindow (&glob);
    myMainWindow->show();

    int ret = app.exec();

    rhea::deinit();
    return ret;
}
