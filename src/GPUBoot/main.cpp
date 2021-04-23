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
    QProcess proc;
    proc.setProgram(APPLICATION_FOLDER + "/GPUPackage2019/GPUFusion");
    proc.startDetached();
}

//*********************************************************
int main(int argc, char *argv[])
{
    //recupera il path corrente
    //usa la malloc per allocare il path
    {
        char *appPathNoSlash = get_current_dir_name();
        APPLICATION_FOLDER = appPathNoSlash;
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
