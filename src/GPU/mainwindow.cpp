#include "header.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "history.h"




//********************************************************************************
MainWindow::MainWindow() :
        QMainWindow(NULL),
        ui(new Ui::MainWindow)
{
    isInterruptActive=false;

    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    utils::hideMouse();

    //Settaggi del browser
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::AutoLoadImages, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);

    //espando la webView a tutto schermo
    ui->webView->setGeometry(0,0,ScreenW,ScreenH);
    ui->webView->setMouseTracking(false);
    ui->webView->raise();
    ui->webView->settings()->setAttribute(QWebSettings::PluginsEnabled,true);
    ui->webView->settings()->setAttribute(QWebSettings::AutoLoadImages, true);
    ui->webView->settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);
    ui->webView->setEnabled(false);

#ifdef _DEBUG
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    ui->webView->setEnabled(true);
    ui->webView->setFocus();
#endif



    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerInterrupt()));
    timer->start(TIMER_INTERVAL_MSEC);


    /*if (utils::isUsbPresent() == true)
        setFormStatus(FormStatus_BOOT, "costruttore");
    else*/
        priv_loadGUIFirstPage();

    ui->webView->setFocus();
}


//********************************************************************************
MainWindow::~MainWindow()
{
    delete ui;
}

//******************************************************
void MainWindow::priv_loadGUIFirstPage()
{
    QString url = "file://" + utils::getFolder_GUI() + "/web/startup.html";
    ui->webView->load(QUrl(url));
}


//*****************************************************
void MainWindow::timerInterrupt()
{
    if (isInterruptActive)
        return;
    isInterruptActive=true;

    isInterruptActive=false;
}


