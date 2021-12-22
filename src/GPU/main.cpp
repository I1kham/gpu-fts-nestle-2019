#include "header.h"
#include "mainwindow.h"
#include <QApplication>
#include <QDir>
//#include <QtWebView/QtWebView>
#include "../CPUBridge/CPUChannelCom.h"
#include "../CPUBridge/CPUChannelFakeCPU.h"
#include "../SocketBridge/SocketBridge.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"
#include "../rheaExternalSerialAPI/ESAPI.h"
#include "../rheaCommonLib/SimpleLogger/FileLogger.h"

//file:///home/root/Desktop/gpu-fts-nestle-2019/bin/current/gui/web/startup.html
static MainWindow *myMainWindow = NULL;

#define		GPU_SUBSCRIBER_UID                  0x2F06

//****************************************************
bool subscribeToCPU (const HThreadMsgW hCPUServiceChannelW, cpubridge::sSubscriber *out_subscriber)
{
    bool ret = false;

    //creo una msgQ temporanea per ricevere da CPUBridge la risposta alla mia richiesta di iscrizione
    HThreadMsgR hMsgQR;
    HThreadMsgW hMsgQW;
    rhea::thread::createMsgQ (&hMsgQR, &hMsgQW);

    //invio la richiesta
    cpubridge::subscribe (hCPUServiceChannelW, hMsgQW, GPU_SUBSCRIBER_UID);

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
bool subscribeToESAPI (cpubridge::sSubscriber *out_subscriber)
{
    bool ret = false;

    //creo una msgQ temporanea per ricevere da CPUBridge la risposta alla mia richiesta di iscrizione
    HThreadMsgR hMsgQR;
    HThreadMsgW hMsgQW;
    rhea::thread::createMsgQ (&hMsgQR, &hMsgQW);

    //invio la richiesta
    esapi::subscribe (hMsgQW);

    //attendo risposta
    u64 timeToExitMSec = rhea::getTimeNowMSec() + 2000;
    do
    {
        rhea::thread::sleepMSec(50);

        rhea::thread::sMsg msg;
        if (rhea::thread::popMsg(hMsgQR, &msg))
        {
            //ok, ci siamo
            if (msg.what == ESAPI_SERVICECH_SUBSCRIPTION_ANSWER)
            {
                esapi::translate_SUBSCRIPTION_ANSWER (msg, out_subscriber);
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


//*****************************************************
bool startCPUBridge (HThreadMsgW *hCPUServiceChannelW, rhea::ISimpleLogger *logger)
{
#if defined(PLATFORM_YOCTO_EMBEDDED)
    //apro un canale di comunicazione con la CPU fisica sulla porta seriale COM_PORT (vedi header.h)
    cpubridge::CPUChannelFakeCPU *chToCPU = new cpubridge::CPUChannelFakeCPU(); bool b = chToCPU->open (logger);
#elif defined(PLATFORM_ROCKCHIP)
    #ifdef _DEBUG
        //apro un canale di comunicazione con una finta CPU
        cpubridge::CPUChannelFakeCPU *chToCPU = new cpubridge::CPUChannelFakeCPU(); bool b = chToCPU->open (logger);

        //cpubridge::CPUChannelCom *chToCPU = new cpubridge::CPUChannelCom(); bool b = chToCPU->open("dev/ttyUSB0", logger);
        //cpubridge::CPUChannelCom *chToCPU = new cpubridge::CPUChannelCom(); bool b = chToCPU->open("/dev/ttyS0", logger);
    #else
        //apro un canale con la CPU fisica
        cpubridge::CPUChannelCom *chToCPU = new cpubridge::CPUChannelCom(); bool b = chToCPU->open(CPU_COMPORT, logger);
        //cpubridge::CPUChannelFakeCPU *chToCPU = new cpubridge::CPUChannelFakeCPU(); bool b = chToCPU->open (logger);
    #endif
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


    if (!cpubridge::startServer(chToCPU, logger, &hCPUThread, hCPUServiceChannelW))
        return false;

    //starto socketBridge che a sua volta si iscrivera'  a CPUBridge
    rhea::HThread hSocketBridgeThread;
    socketbridge::startServer(logger, *hCPUServiceChannelW, false, true, &hSocketBridgeThread);

    return true;
}

//*****************************************************
bool executeShellCommandAndStoreResult (const char *shellCommand, char *out_result, u32 sizeOfOutResult)
{
    FILE *fp = popen (shellCommand, "r");
    if (NULL == fp)
        return false;
    fgets (out_result, sizeOfOutResult, fp);
    fclose (fp);
    return true;
}

/****************************************************
 * Filla [glob] con i path dei vari folder utilizzati dalla GPU
 */
void setupFolderInformation (sGlobal *glob)
{
    u8 s[1024];
    rhea::Allocator *allocator = rhea::getSysHeapAllocator();

    //local folders
    const u8 *baseLocalFolder = rhea::getPhysicalPathToAppFolder();

    rhea::string::utf8::spf (s, sizeof(s), "%s/temp", baseLocalFolder);
    glob->tempFolder = rhea::string::utf8::allocStr(allocator, s);


    rhea::string::utf8::spf (s, sizeof(s), "%s/current", baseLocalFolder);
    glob->current = rhea::string::utf8::allocStr(allocator, s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/gui", glob->current);
    glob->current_GUI = rhea::string::utf8::allocStr(allocator, s);
    rhea::fs::folderCreate(s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/lang", glob->current);
    glob->current_lang = rhea::string::utf8::allocStr(allocator, s);
    rhea::fs::folderCreate(s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/da3", glob->current);
    glob->current_da3 = rhea::string::utf8::allocStr(allocator, s);
    rhea::fs::folderCreate(s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/last_installed/da3", baseLocalFolder);
    glob->last_installed_da3 = rhea::string::utf8::allocStr(allocator, s);
    rhea::fs::folderCreate(s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/last_installed/cpu", baseLocalFolder);
    glob->last_installed_cpu = rhea::string::utf8::allocStr(allocator, s);
    rhea::fs::folderCreate(s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/last_installed/manual", baseLocalFolder);
    glob->last_installed_manual = rhea::string::utf8::allocStr(allocator, s);
    rhea::fs::folderCreate(s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/last_installed/gui", baseLocalFolder);
    glob->last_installed_gui = rhea::string::utf8::allocStr(allocator, s);
    rhea::fs::folderCreate(s);

    sprintf_s ((char*)s, sizeof(s), "%s/autoUpdate", baseLocalFolder);
    glob->localAutoUpdateFolder = rhea::string::utf8::allocStr(allocator, s);
    rhea::fs::folderCreate(s);



    //USB folders
    u8 baseUSBFolder[256];
#if defined(PLATFORM_YOCTO_EMBEDDED)
    rhea::string::utf8::spf (baseUSBFolder, sizeof(baseUSBFolder), USB_MOUNTPOINT);
#elif defined(PLATFORM_ROCKCHIP)
    //nel caso della SECO, la cartella /media/SDA1 esiste sempre perchè fa parte del FS.
    //Per capire se c'è o no la chiavetta, devo invocare la shell usando "mount | grep -c sda1". Questo comando ritorna 0 se non c'è chiavetta montata, 1 se c'è.
    //Se non c'è, metto un percorso sbagliato in baseUSBFolder per simulare il fatto che la cartella preposta ad ospitare
    //la chiave USB non esiste

    //const int isUSBMounted = system("mount | grep -c sda1");
    char isUSBMounted[8];
    memset (isUSBMounted, 0, sizeof(isUSBMounted));
    executeShellCommandAndStoreResult ("/bin/mount | /bin/grep -c sda1", isUSBMounted, sizeof(isUSBMounted));
    if (isUSBMounted[0] == '1')
        rhea::string::utf8::spf (baseUSBFolder, sizeof(baseUSBFolder), USB_MOUNTPOINT);
    else
        rhea::string::utf8::spf (baseUSBFolder, sizeof(baseUSBFolder), "xxx");
#else
    rhea::string::utf8::spf (baseUSBFolder, sizeof(baseUSBFolder), "%s/simula-chiavetta-usb", baseLocalFolder);
    //sprintf_s (baseUSBFolder, sizeof(baseUSBFolder), "%s/pippo", baseLocalFolder);
#endif

    rhea::string::utf8::spf (s, sizeof(s), "%s/rhea", baseUSBFolder);
    glob->usbFolder = rhea::string::utf8::allocStr(allocator, s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/rheaData", glob->usbFolder);
    glob->usbFolder_VMCSettings = rhea::string::utf8::allocStr(allocator, s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/rheaFirmwareCPU01", glob->usbFolder);
    glob->usbFolder_CPUFW = rhea::string::utf8::allocStr(allocator, s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/rheaGUI", glob->usbFolder);
    glob->usbFolder_GUI = rhea::string::utf8::allocStr(allocator, s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/rheaDataAudit", glob->usbFolder);
    glob->usbFolder_Audit = rhea::string::utf8::allocStr(allocator, s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/lang", glob->usbFolder);
    glob->usbFolder_Lang = rhea::string::utf8::allocStr(allocator, s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/rheaManual", glob->usbFolder);
    glob->usbFolder_Manual = rhea::string::utf8::allocStr(allocator, s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/AUTOF2", glob->usbFolder);
    glob->usbFolder_AutoF2 = rhea::string::utf8::allocStr(allocator, s);


    //vediamo se il folder della USB esiste
    if (rhea::fs::folderExists(baseUSBFolder))
    {
        //se non esiste già, creo la cartella rhea su chiave USB
        if (!rhea::fs::folderExists(glob->usbFolder))
            rhea::fs::folderCreate(glob->usbFolder);
    }
}

void unsetupFolderInformation (sGlobal *glob)
{
    rhea::Allocator *allocator = rhea::getSysHeapAllocator();

    RHEAFREE(allocator, glob->tempFolder);
    RHEAFREE(allocator, glob->current);
    RHEAFREE(allocator, glob->current_GUI);
    RHEAFREE(allocator, glob->current_lang);
    RHEAFREE(allocator, glob->current_da3);
    RHEAFREE(allocator, glob->last_installed_da3);
    RHEAFREE(allocator, glob->last_installed_cpu);
    RHEAFREE(allocator, glob->last_installed_manual);
    RHEAFREE(allocator, glob->last_installed_gui);
    RHEAFREE(allocator, glob->localAutoUpdateFolder);
    RHEAFREE(allocator, glob->usbFolder);
    RHEAFREE(allocator, glob->usbFolder_VMCSettings);
    RHEAFREE(allocator, glob->usbFolder_CPUFW);
    RHEAFREE(allocator, glob->usbFolder_GUI);
    RHEAFREE(allocator, glob->usbFolder_Audit);
    RHEAFREE(allocator, glob->usbFolder_Lang);
    RHEAFREE(allocator, glob->usbFolder_Manual);
    RHEAFREE(allocator, glob->usbFolder_AutoF2);
}

/****************************************************
 * creo un file di testo in current/gpu per riportare l'attuale versione della GPU
 * Questo file mi serve perchè esiste un comando ajax che chiede la versione GPU e dato che la versione
 * GPU è nell'header.h di questo progetto, SocketBridge non avrebbe modo di conoscerla. Creo il file
 * current/gpu/ver.txt in modo che SocketBridge possa leggerlo e rispondere
 */
void createGPUVerFile (const sGlobal *glob)
{
    u8 s[256];
    sprintf_s ((char*)s, sizeof(s), "%s/gpu/ver.txt", glob->current);
    FILE *f = rhea::fs::fileOpenForWriteBinary (s);
    if (NULL != f)
    {
        sprintf_s ((char*)s, sizeof(s), "%s", GPU_VERSION);
        rhea::fs::fileWrite (f, s, rhea::string::utf8::lengthInBytes(s));
        rhea::fs::fileClose (f);
    }
}

//****************************************************
void run(int argc, char *argv[])
{
    sGlobal glob;
    memset(&glob, 0, sizeof(glob));
    glob.esapiModule.moduleType = esapi::eExternalModuleType::none;
    glob.bIsMilkerAlive = 1;

    //creazione del logger
#ifdef _DEBUG
    glob.logger = new rhea::StdoutLogger();

#else
#if defined(PLATFORM_YOCTO_EMBEDDED) || defined(PLATFORM_ROCKCHIP)
    glob.logger = new rhea::NullLogger();
    //u8 s[256]; sprintf_s ((char*)s, sizeof(s), "%s/output.log", rhea::getPhysicalPathToAppFolder()); glob.logger = new rhea::FileLogger(s);
#else
    glob.logger = new rhea::NullLogger();

#endif
#endif

    //recupero informazioni sui vari folder
    setupFolderInformation (&glob);
    glob.logger->log ("current folder is: %s\n", rhea::getPhysicalPathToAppFolder());

    createGPUVerFile (&glob);

    //Avvio della SMU
    HThreadMsgW hCPUServiceChannelW;
    startCPUBridge (&hCPUServiceChannelW, glob.logger);

    //Mi iscrivo alla CPU per ricevere direttamente le notifiche che questa manda al cambiare del suo stato
    subscribeToCPU (hCPUServiceChannelW, &glob.cpuSubscriber);

    //starto ESAPI
    rhea::HThread hThreadESAPI;
    if (esapi::startThread (ESAPI_COMPORT, hCPUServiceChannelW, glob.logger, &hThreadESAPI))
    {
        //Mi iscrivo a ESAPI per ricevere direttamente le notifiche che questa manda al cambiare del suo stato
        subscribeToESAPI (&glob.esapiSubscriber);
    }

    //faccio partire RSProto per la telemetria con SECO
#ifdef _DEBUG
    rhea::shell_runCommandNoWait ("./UBUNTU_DEBUG_SecoBridge 127.0.0.1 2283");
#else
    #ifdef PLATFORM_YOCTO_EMBEDDED
        rhea::shell_runCommandNoWait ("./RSProto 127.0.0.1 2283");
    #endif
#endif

    //Avvio del main form
    //QtWebView::initialize();
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
#if defined(PLATFORM_ROCKCHIP)
    //sposto la "working directory" nel path di questo exe
    if (argc > 0)
    {
        u8 fullpath[256];
        if (argv[0][0] == '.')
        {
            char *curPath = get_current_dir_name();
            rhea::string::utf8::spf (fullpath, sizeof(fullpath), "%s%s", curPath, &argv[0][1]);
            free(curPath);
        }
        else
        {
            //recupero il path direttamente da argv[0]
            rhea::fs::extractFilePathWithOutSlash (reinterpret_cast<const u8*>(argv[0]), fullpath, sizeof(fullpath));
        }
        chdir(reinterpret_cast<const char*>(fullpath));
    }
#endif

    rhea::init("rheaGPU", NULL);

    run (argc, argv);

    rhea::deinit();
    return 0;
}
