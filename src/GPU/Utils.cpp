#include "Utils.h"
#include <stdio.h>
#include <string.h>
#include "header.h"
#include <QDir>
#include <QFile>


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

//****************************************************
void utils::DEBUG_MSG (const char* format, ...)
{}


//****************************************************
void utils::hideMouse()
{
    #ifndef DEBUG_SHOW_MOUSE
        QApplication::setOverrideCursor(Qt::BlankCursor);
    #endif
}

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





/****************************************************
 * double updateCPUStats()
 *
 * ritorna la % di utilizzo attuale della CPU
 */
struct sCPUStats
{
    long double v[8];
    double loadavg;
    unsigned char i;
    unsigned long timerMsec;
};
sCPUStats cpuStats;

double utils::updateCPUStats(unsigned long timeSinceLastCallMSec)
{
    if (cpuStats.i == 0)
        cpuStats.i = 1;
    else
        cpuStats.i = 0;

    unsigned char index = cpuStats.i * 4;

    FILE *f = fopen("/proc/stat","r");
    fscanf(f,"%*s %Lf %Lf %Lf %Lf",&cpuStats.v[index],&cpuStats.v[index+1],&cpuStats.v[index+2],&cpuStats.v[index+3]);
    fclose(f);

    if (cpuStats.i == 1)
    {
        cpuStats.timerMsec += timeSinceLastCallMSec;
        if (cpuStats.timerMsec >= 250)
        {
            cpuStats.timerMsec = 0;
            long double a3 = cpuStats.v[0] + cpuStats.v[1] + cpuStats.v[2];
            long double b3 = cpuStats.v[4] + cpuStats.v[5] + cpuStats.v[6];
            long double d = (b3 + cpuStats.v[7]) - (a3 + cpuStats.v[3]);
            if (d != 0)
            {
                cpuStats.loadavg = (double) ((b3 - a3) / d);
                if (cpuStats.loadavg <= 0)
                    cpuStats.loadavg = 0;
            }
        }
    }

    return cpuStats.loadavg;
}

//****************************************************
bool utils::copyRecursively (const QString &srcFilePath, const QString &tgtFilePath)
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir())
    {
        QDir targetDir(tgtFilePath);
        targetDir.cdUp();
        if (!targetDir.mkdir(QFileInfo(tgtFilePath).fileName()))
        {
            //QMessageBox::information(NULL, "t_A", tgtFilePath);
            return false;
        }


        QDir sourceDir(srcFilePath);
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        foreach (const QString &fileName, fileNames)
        {
            const QString newSrcFilePath= srcFilePath + QLatin1Char('/') + fileName;
            const QString newTgtFilePath= tgtFilePath + QLatin1Char('/') + fileName;
            if (!copyRecursively(newSrcFilePath, newTgtFilePath))
            {
                return false;
            }
        }
    }
    else
    {
        if (!QFile::copy(srcFilePath, tgtFilePath))
            return false;
    }

    return true;
}
