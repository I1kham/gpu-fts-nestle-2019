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
    return socketbridge::startServer(logger, hCPUServiceChannelW, false, out_hThread);
}


//*****************************************************
bool startCPUBridge (HThreadMsgW *hCPUServiceChannelW, rhea::ISimpleLogger *logger)
{
#ifdef PLATFORM_YOCTO_EMBEDDED
    //apro un canale di comunicazione con la CPU fisica
    cpubridge::CPUChannelCom *chToCPU = new cpubridge::CPUChannelCom();
    bool b = chToCPU->open(CPU_COMPORT, logger);
#else
    //apro un canale di comunicazione con una finta CPU
    //cpubridge::CPUChannelFakeCPU *chToCPU = new cpubridge::CPUChannelFakeCPU(); bool b = chToCPU->open (logger);

    //apro un canale con la CPU fisica
    cpubridge::CPUChannelCom *chToCPU = new cpubridge::CPUChannelCom();    bool b = chToCPU->open(CPU_COMPORT, logger);

#endif

    if (!b)
        return false;

    //creo il thread di CPUBridge
    rhea::HThread hCPUThread;


    if (!cpubridge::startServer(chToCPU, logger, &hCPUThread, hCPUServiceChannelW))
        return false;

    //starto socketBridge che a sua volta siiscrivera'  a CPUBridge
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
    const char *baseLocalFolder = rhea::getPhysicalPathToAppFolder();

    sprintf_s (s, sizeof(s), "%s/temp", baseLocalFolder);
    glob->tempFolder = rhea::string::alloc(allocator, s);


    sprintf_s (s, sizeof(s), "%s/current", baseLocalFolder);
    glob->current = rhea::string::alloc(allocator, s);

    sprintf_s (s, sizeof(s), "%s/gui", glob->current);
    glob->current_GUI = rhea::string::alloc(allocator, s);
    rhea::fs::folderCreate(s);

    sprintf_s (s, sizeof(s), "%s/lang", glob->current);
    glob->current_lang = rhea::string::alloc(allocator, s);
    rhea::fs::folderCreate(s);

    sprintf_s (s, sizeof(s), "%s/da3", glob->current);
    glob->current_da3 = rhea::string::alloc(allocator, s);
    rhea::fs::folderCreate(s);

    sprintf_s (s, sizeof(s), "%s/last_installed/da3", baseLocalFolder);
    glob->last_installed_da3 = rhea::string::alloc(allocator, s);
    rhea::fs::folderCreate(s);

    sprintf_s (s, sizeof(s), "%s/last_installed/cpu", baseLocalFolder);
    glob->last_installed_cpu = rhea::string::alloc(allocator, s);
    rhea::fs::folderCreate(s);

    sprintf_s (s, sizeof(s), "%s/last_installed/manual", baseLocalFolder);
    glob->last_installed_manual = rhea::string::alloc(allocator, s);
    rhea::fs::folderCreate(s);

    sprintf_s (s, sizeof(s), "%s/last_installed/gui", baseLocalFolder);
    glob->last_installed_gui = rhea::string::alloc(allocator, s);
    rhea::fs::folderCreate(s);




    //USB folders
    char baseUSBFolder[256];
#ifdef PLATFORM_YOCTO_EMBEDDED
    sprintf_s (baseUSBFolder, sizeof(baseUSBFolder), USB_MOUNTPOINT);
#else
    sprintf_s (baseUSBFolder, sizeof(baseUSBFolder), "%s/simula-chiavetta-usb", baseLocalFolder);
    //sprintf_s (baseUSBFolder, sizeof(baseUSBFolder), "%s/pippo", baseLocalFolder);
#endif

    sprintf_s (s, sizeof(s), "%s/rhea", baseUSBFolder);
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

    sprintf_s (s, sizeof(s), "%s/rheaManual", glob->usbFolder);
    glob->usbFolder_Manual = rhea::string::alloc(allocator, s);

    sprintf_s (s, sizeof(s), "%s/AUTOF2", glob->usbFolder);
    glob->usbFolder_AutoF2 = rhea::string::alloc(allocator, s);


    //vediamo se il folder della USB esiste
    if (rhea::fs::folderExists(baseUSBFolder))
    {
        //se non esiste gi�, creo la cartella rhea su chiave USB
        if (!rhea::fs::folderExists(glob->usbFolder))
            rhea::fs::folderCreate(glob->usbFolder);
    }
}

void unsetupFolderInformation (sGlobal *glob)
{
    rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();

    RHEAFREE(allocator, glob->tempFolder);
    RHEAFREE(allocator, glob->current);
    RHEAFREE(allocator, glob->current_GUI);
    RHEAFREE(allocator, glob->current_lang);
    RHEAFREE(allocator, glob->current_da3);
    RHEAFREE(allocator, glob->last_installed_da3);
    RHEAFREE(allocator, glob->last_installed_cpu);
    RHEAFREE(allocator, glob->last_installed_manual);
    RHEAFREE(allocator, glob->last_installed_gui);
    RHEAFREE(allocator, glob->usbFolder);
    RHEAFREE(allocator, glob->usbFolder_VMCSettings);
    RHEAFREE(allocator, glob->usbFolder_CPUFW);
    RHEAFREE(allocator, glob->usbFolder_GUI);
    RHEAFREE(allocator, glob->usbFolder_Audit);
    RHEAFREE(allocator, glob->usbFolder_Lang);
    RHEAFREE(allocator, glob->usbFolder_Manual);
    RHEAFREE(allocator, glob->usbFolder_AutoF2);
}

//****************************************************
void run(int argc, char *argv[])
{
    sGlobal glob;
    memset(&glob, 0, sizeof(glob));

    //creazione del logger
#ifdef _DEBUG
    glob.logger = new rhea::StdoutLogger();
#else
    glob.logger = new rhea::NullLogger();
#endif

    //recupero informazioni sui vari folder
    setupFolderInformation (&glob);
    glob.logger->log ("current folder is: %s\n", rhea::getPhysicalPathToAppFolder());

    //Avvio della SMU
    HThreadMsgW hCPUServiceChannelW;
    startCPUBridge (&hCPUServiceChannelW, glob.logger);

    //Mi iscrivo alla CPU per ricevere direttamente le notifiche che questa manda al cambiare del suo stato
    subscribeToCPU (hCPUServiceChannelW, &glob.subscriber);


    //Avvio del main form
    QApplication app(argc, argv);
    utils::hideMouse();

    myMainWindow = new MainWindow (&glob);
    myMainWindow->show();

    app.exec();

    unsetupFolderInformation(&glob);
}


//****************************************************
int main (int argc, char *argv[])
{
    rhea::init("rheaGPU", NULL);

    run (argc, argv);

    rhea::deinit();
    return 0;
}
