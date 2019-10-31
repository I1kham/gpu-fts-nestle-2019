#ifndef RHEA_HEADER_H
#define RHEA_HEADER_H


//Versione GPU (Fusioe Beta v2)
#define GPU_VERSION_MAJOR   3
#define GPU_VERSION_MINOR   0
#define GPU_VERSION_BUILD   0


//nome della porta seriale
#ifdef PLATFORM_UBUNTU_DESKTOP
    #define CPU_COMPORT  "/dev/ttyUSB0"
#else
    #define CPU_COMPORT     "/dev/ttymxc3"
    #define USB_MOUNTPOINT  "/run/media/sda1"
#endif


//se questa è definita, premento il bottone prog (quello fisico), si va direttamente nel vecchio menu prog
#undef BTN_PROG_VA_IN_VECCHIO_MENU_PROGRAMMAZIONE


#include "../rheaCommonLib/rhea.h"
#include "../CPUBridge/CPUBridge.h"
#include "Utils.h"


/****************************************************+
 *
 */
struct sGlobal
{
    cpubridge::sSubscriber subscriber;

    char  cpuVersion[128];

    char *tempFolder;

    char *current;
    char *current_GUI;
    char *current_lang;
    char *current_da3;
    char *last_installed_da3;
    char *last_installed_cpu;
    char *last_installed_manual;
    char *last_installed_gui;

    char *usbFolder;                    //path di base verso il folder rhea su chiavetta USB (NULL se la chiavetta USB non esiste)
    char *usbFolder_VMCSettings;        //folder su chiavetta USB per i da3
    char *usbFolder_CPUFW;              //folder su chiavetta USB per il fw di CPU
    char *usbFolder_GUI;                //folder su chiavetta USB per le GUI
    char *usbFolder_Audit;              //folder su chiavetta USB per salvare i data audit
    char *usbFolder_Lang;               //folder su chiavetta USB per il multilanguage

    rhea::ISimpleLogger *logger;
};



#endif 

