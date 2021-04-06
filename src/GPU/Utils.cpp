#include "Utils.h"
#include <stdio.h>
#include <string.h>
#include "header.h"
#include <QDir>
#include <QFile>
#include <QApplication>



//****************************************************
void utils::hideMouse()
{
#ifdef PLATFORM_UBUNTU_DESKTOP
    return;
#else
    #ifndef _DEBUG
        QApplication::setOverrideCursor(Qt::BlankCursor);
    #endif
#endif
}

//****************************************************
void utils::waitAndProcessEvent (u32 msecToWait)
{
    const u64 timeToExit = rhea::getTimeNowMSec() + (u64)msecToWait;
    while (rhea::getTimeNowMSec() < timeToExit)
    {
        QApplication::processEvents();
        rhea::thread::sleepMSec(20);
    }
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
        //DEBUG_MSG   ("FONT: using DejaVu Sans");
        out.setFamily("DejaVu Sans");
        out.setPointSize(pointSize);
    }
    else
    {
        //DEBUG_MSG   ("FONT: Noto Sans CJK SC");
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
    rhea::fs::fileClose(f);

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
