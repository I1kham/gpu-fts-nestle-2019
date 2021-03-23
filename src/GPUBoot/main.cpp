#include "mainwindow.h"
#include <QApplication>
#include "main.h"
#include <qdir.h>
#include <qprocess.h>


QString APPLICATION_FOLDER("/home/seco/Desktop/rhea");

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
    //vediamo se esiste una chiavetta USB con dentro le cartelle corrette
    QDir folder (RHEA_USB_FOLDER);
    if (folder.exists())
    {
        QApplication a(argc, argv);
        MainWindow w;
        w.show();
        a.exec();
    }

    startGPU();
    return 0;
}
