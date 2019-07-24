#include "Utils.h"
#include <stdio.h>
#include <string.h>
#include "header.h"
#include <QDir>

struct sFolderInfo
{
    QString root;
    QString vmcSettings;
    QString CPU;
    QString GUI;
    QString manual;
    QString languages;

    QString usbPenDrive;
    QString usbPenDrive_vmcSetting;
    QString usbPenDrive_CPU;
    QString usbPenDrive_GUI;
    QString usbPenDrive_Manual;
    QString usbPenDrive_Audit;
    QString usbPenDrive_Lang;
};
sFolderInfo folderInfo;

//*****************************************************
void utils::gatherFolderInfo (const QString &Folder_appGpu)
{
#ifdef PLATFORM_YOCTO_EMBEDDED
    folderInfo.root = Folder_appGpu.left(Folder_appGpu.lastIndexOf("/"));
    folderInfo.usbPenDrive = "/run/media/sda1/rhea";
#else
    folderInfo.root = Folder_appGpu;
    folderInfo.usbPenDrive = Folder_appGpu +"/simula-chiavetta-usb";
#endif

    //folder su HD
    folderInfo.vmcSettings = folderInfo.root + "/rheaData";
    folderInfo.CPU = folderInfo.root + "/rheaFirmwareCPU01";
    folderInfo.GUI = folderInfo.root + "/rheaGUI";
    folderInfo.manual = folderInfo.root + "/rheaManual";
    folderInfo.languages = folderInfo.root + "/lang";

    //folder su chiave usb
    folderInfo.usbPenDrive_vmcSetting = folderInfo.usbPenDrive +"/rheaData";
    folderInfo.usbPenDrive_CPU = folderInfo.usbPenDrive +"/rheaFirmwareCPU01";
    folderInfo.usbPenDrive_GUI = folderInfo.usbPenDrive +"/rheaGUI";
    folderInfo.usbPenDrive_Manual = folderInfo.usbPenDrive +"/rheaManual";
    folderInfo.usbPenDrive_Audit = folderInfo.usbPenDrive +"/rheaDataAudit";
    folderInfo.usbPenDrive_Lang = folderInfo.usbPenDrive +"/lang";
}

//*****************************************************
bool utils::isUsbPresent()                          { return QDir(folderInfo.usbPenDrive).exists(); }
const QString& utils::getFolder_Usb_VMCSettings()   { return folderInfo.usbPenDrive_vmcSetting; }
const QString& utils::getFolder_Usb_CPU()           { return folderInfo.usbPenDrive_CPU; }
const QString& utils::getFolder_Usb_GUI()           { return folderInfo.usbPenDrive_GUI; }
const QString& utils::getFolder_Usb_Manual()        { return folderInfo.usbPenDrive_Manual; }
const QString& utils::getFolder_Usb_Audit()         { return folderInfo.usbPenDrive_Audit; }
const QString& utils::getFolder_Usb_Lang()          { return folderInfo.usbPenDrive_Lang; }

const QString& utils::getFolder_VMCSettings()         { return folderInfo.vmcSettings; }
const QString& utils::getFolder_CPU()                 { return folderInfo.CPU; }
const QString& utils::getFolder_GUI()                 { return folderInfo.GUI; }
const QString& utils::getFolder_Manual()              { return folderInfo.manual; }
const QString& utils::getFolder_Lang()                { return folderInfo.languages; }


//*************************************************************************************
u8 utils::evalChecksum (unsigned char *buf, unsigned int len)
{
    u8 res = 0;
    for (u8 i=0; i<len; i++)
        res += buf[i];
    return res;
}

//****************************************************
char* utils::QCharToCStr (const QChar *in, char *out)
{
    int i=0;
    while (in[i] != 0x00)
    {
        out[i] = in[i].toLatin1();
        i++;
    }
    out[i] =0;
    return out;
}

/*****************************************************
 * Font
 * Il font col charset Latin, JP, chinese è "Noto Sans CJK SC", installato di default nella immagine dell'OS 5
 * Il font col charset Hebrew, è il Roboto, installato di default nella immagine dell'OS 5
 */
void utils::getRightFontForLanguage (QFont &out, int pointSize, const char *iso2LettersLanguageCode)
{
    if (strcasecmp(iso2LettersLanguageCode, "HE") == 0)
    {
        DEBUG_MSG   ("FONT: using DejaVu Sans");
        out.setFamily("DejaVu Sans");
        out.setPointSize(pointSize);
    }
    else
    {
        DEBUG_MSG   ("FONT: Noto Sans CJK SC");
        out.setFamily("Noto Sans CJK SC");
        out.setPointSize(pointSize);
    }
}





