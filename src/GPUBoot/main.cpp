      #include "mainwindow.h"
#include <QApplication>
#include "main.h"
#include <qdir.h>
#include <qprocess.h>
#include <unistd.h>
#include "../rheaCommonLib/rhea.h"


QString APPLICATION_FOLDER("/");

//*********************************************************
void hideMouse()
{
#ifndef _DEBUG
    QApplication::setOverrideCursor(Qt::BlankCursor);
#endif
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

//*****************************************************
void checkForGPUUpdate()
{
#ifdef _DEBUG
    printf ("checkForGPUUpdate\n");
#endif
    u8 s1[512];
    rhea::string::utf8::spf (s1, sizeof(s1), "%s/GPUPackage2019/autoUpdate", rhea::getPhysicalPathToAppFolder());
    if (!rhea::fs::folderExists(s1))
    {
#ifdef _DEBUG
        printf ("%s not found\n", s1);
#endif
        return;
    }

    u8 s2[512];
    if (!rhea::fs::findFirstFileInFolderWithJolly (s1, (const u8*)"*.mh210", true, s2, sizeof(s2)))
    {
#ifdef _DEBUG
        printf ("no GPU update found in %s\n", s1);
#endif
        return;
    }

    //copio l'aggiornamento spostandolo dalla cartella GPUPackage2019/autoUpdate e mettendolo in /
    rhea::string::utf8::spf (s1, sizeof(s1), "%s/update.mh210", rhea::getPhysicalPathToAppFolder());
    rhea::fs::fileCopy (s2, s1);
#ifdef _DEBUG
    printf ("copying %s into %s\n", s2, s1);
#endif

    //elimino il file di update dalla cartella GPUPackage2019/autoUpdate
#ifdef _DEBUG
    printf ("deleting %s\n", s2);
#endif
    rhea::fs::fileDelete (s2);


    //unzippo
    rhea::string::utf8::spf (s2, sizeof(s2), "tar -xf %s", s1);
#ifdef _DEBUG
    printf ("%s\n", s2);
#endif
    char result[32];
    memset (result, 0, sizeof(result));
    executeShellCommandAndStoreResult ((const char*)s2, result, sizeof(result));
}

//*********************************************************
void startGPU()
{
    /*QProcess proc;
    proc.setProgram(APPLICATION_FOLDER + "/GPUPackage2019/GPUFusion");
    proc.startDetached();
    */

    char *appPathNoSlash = get_current_dir_name();
    char exeAndPathName[512];

    sprintf (exeAndPathName, "%s/GPUPackage2019", appPathNoSlash);
    printf ("changing dir to %s\n", exeAndPathName);
    chdir(exeAndPathName);

    sprintf (exeAndPathName, "%s/GPUPackage2019/GPUFusion", appPathNoSlash);

    char *argv[4];
    memset (argv, 0, sizeof(argv));
    argv[0] = exeAndPathName;
    printf ("launcing %s\n", exeAndPathName);
    execvp (exeAndPathName, (char* const*)argv);
}

//*********************************************************
int main(int argc, char *argv[])
{
    //sposto la "working directory" nel path di questo exe
    if (argc > 0)
    {
        if (argv[0][0] != '.')
        {
            //recupero il path direttamente da argv[0]
            u8 path[256];
            rhea::fs::extractFilePathWithOutSlash ((const u8*)argv[0], path, sizeof(path));
            chdir((const char*)path);
        }
    }

    rhea::init("rheaGPUBoot", NULL);

    //recupera il path corrente
    //usa la malloc per allocare il path
    {
        char *appPathNoSlash = get_current_dir_name();
        APPLICATION_FOLDER = appPathNoSlash;
        printf ("current folder is: %s\n", appPathNoSlash);
        free(appPathNoSlash);
    }

    //vediamo se esiste un file di aggiornamento della GPU nella cartella autoUpdate
    checkForGPUUpdate();

    //vediamo se esiste una chiavetta USB con dentro le cartelle corrette
    QDir folder (RHEA_USB_FOLDER);
    if (folder.exists())
    {
        QApplication a(argc, argv);
        APPLICATION_FOLDER = QApplication::applicationDirPath();

        MainWindow w;
        w.show();
        a.exec();
    }

    rhea::deinit();
    startGPU();
    return 0;
}
