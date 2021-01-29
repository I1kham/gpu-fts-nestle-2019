#ifndef RHEA_HEADER_H
#define RHEA_HEADER_H


//Versione GPU
#define GPU_VERSION     "2.4.5"


//nome della porta seriale
#ifdef PLATFORM_UBUNTU_DESKTOP
    #define CPU_COMPORT  "/dev/ttyUSB0"
    #define ESAPI_COMPORT  "/dev/ttyUSB1"
#else
    #define CPU_COMPORT     "/dev/ttymxc3"
    #define ESAPI_COMPORT   "/dev/ttymxc2"
    #define USB_MOUNTPOINT  "/run/media/sda1"
#endif


//se questa è definita, premento il bottone prog (quello fisico), si va direttamente nel vecchio menu prog
#undef BTN_PROG_VA_IN_VECCHIO_MENU_PROGRAMMAZIONE


#include "../rheaCommonLib/rhea.h"
#include "../CPUBridge/CPUBridge.h"
#include "Utils.h"
#include "../CPUBridge/DA3.h"

enum eRetCode
{
    eRetCode_none = 0,
    eRetCode_gotoFormBrowser = 1,
    eRetCode_gotoFormOldMenuProg = 2,
    eRetCode_gotoFormBoot = 3,
    eRetCode_gotoNewMenuProgrammazione = 4,
    eRetCode_gotoNewMenuProg_LavaggioSanitario = 5,
    eRetCode_gotoNewMenuProg_lavaggioMilker = 6
};

struct sModuleESAPI
{
    esapi::eExternalModuleType moduleType;
    u8  verMajor;
    u8  verMinor;
};

/****************************************************+
 *
 */
struct sGlobal
{
    cpubridge::sSubscriber cpuSubscriber;
    cpubridge::sSubscriber esapiSubscriber;

    bool            bSyncWithCPUResult;
    char            cpuVersion[128];
    cpubridge::sExtendedCPUInfo extendedCPUInfo;
    u8              bCPUEnteredInMainLoop;
    u8              bIsMilkerAlive;
    sModuleESAPI    esapiModule;

    u16     sendASAP_resetCoffeeGroundDecounter;        //se !=0, il relativo comando viene mandato alla CPU non appena questa è pronta a riceverlo

    u8      *tempFolder;

    u8      *current;
    u8      *current_GUI;
    u8      *current_lang;
    u8      *current_da3;
    u8      *last_installed_da3;
    u8      *last_installed_cpu;
    u8      *last_installed_manual;
    u8      *last_installed_gui;

    u8      *usbFolder;                    //path di base verso il folder rhea su chiavetta USB (NULL se la chiavetta USB non esiste)
    u8      *usbFolder_VMCSettings;        //folder su chiavetta USB per i da3
    u8      *usbFolder_CPUFW;              //folder su chiavetta USB per il fw di CPU
    u8      *usbFolder_GUI;                //folder su chiavetta USB per le GUI
    u8      *usbFolder_Audit;              //folder su chiavetta USB per salvare i data audit
    u8      *usbFolder_Lang;               //folder su chiavetta USB per il multilanguage
    u8      *usbFolder_Manual;
    u8      *usbFolder_AutoF2;              //folder su chiavetta USB per l'auto update

    rhea::ISimpleLogger *logger;
};


void activeSleep (u32 howMuchMSec);

#endif 

