#include "mainwindow.h"
#include <QApplication>
#include "main.h"
#include <qdir.h>
#include <qprocess.h>
#include <unistd.h>


QString APPLICATION_FOLDER("/");

//*********************************************************
void hideMouse()
{
#ifndef _DEBUG
    QApplication::setOverrideCursor(Qt::BlankCursor);
#endif
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
    //recupera il path corrente
    //usa la malloc per allocare il path
    {
        char *appPathNoSlash = get_current_dir_name();
        APPLICATION_FOLDER = appPathNoSlash;
        printf ("current folder is: %s\n", appPathNoSlash);
        free(appPathNoSlash);
    }


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

    startGPU();
    return 0;
}
