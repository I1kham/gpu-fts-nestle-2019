#include "header.h"
#include <QTimer>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "formboot.h"
#include "FormBrowser.h"
#include "formprog.h"



//********************************************************************************
MainWindow::MainWindow (const sGlobal *globIN) :
        QMainWindow(NULL),
        ui(new Ui::MainWindow)
{
    glob = globIN;

    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    utils::hideMouse();



    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerInterrupt()));
    timer->start(100);
}


//********************************************************************************
MainWindow::~MainWindow()
{
    delete ui;
}

//*****************************************************
void MainWindow::timerInterrupt()
{
    timer->stop();
    delete timer;
    timer = NULL;
    utils::hideMouse();

    //siamo alla prima visualizzazione di questa finestra.
    //Se c'è la chiavetta USB, andiamo in frmBoot, altrimenti direttamente in frmBrowser
    if (NULL != glob->usbFolder)
    {
        FormBoot *frm = new FormBoot(this);
        frm->exec();
        delete frm;
    }

    while (1)
    {
        //visualizziamo frm browser
        FormBrowser *frmBrowser = new FormBrowser(this, glob);
        frmBrowser->exec();
        delete frmBrowser;

        //se usciamo da frmBrowser, è per visualizzare frmProg dato che non è previsto un "quit"
        FormProg *frmProg = new FormProg(this, glob);
        frmProg->exec();
        delete frmProg;
    }
}
