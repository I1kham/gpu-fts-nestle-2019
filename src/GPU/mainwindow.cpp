#include "header.h"
#include <QTimer>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "history.h"
#include "../rheaAppLib/rheaAppUtils.h"



//********************************************************************************
MainWindow::MainWindow (sGlobal *globIN) :
        QMainWindow(NULL),
        ui(new Ui::MainWindow)
{
    glob = globIN;
    retCode = 0;

    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    utils::hideMouse();

    //Settaggi del browser
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::AutoLoadImages, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);

    //espando la webView a tutto schermo
    ui->webView->setVisible(false);
    ui->webView->setMouseTracking(false);
    ui->webView->move(0,0);
    ui->webView->resize(1024, 600);
    ui->webView->settings()->setAttribute(QWebSettings::PluginsEnabled,true);
    ui->webView->settings()->setAttribute(QWebSettings::AutoLoadImages, true);
    ui->webView->settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);

#ifdef _DEBUG
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
#endif



    //istanzio il form boot se necessario
    frmBoot = NULL;
    if (NULL != glob->usbFolder)
    {
        frmBoot = new FormBoot(this, glob);
        frmBoot->hide();
    }

    frmProg = NULL;


    priv_scheduleFormChange (eForm_main_syncWithCPU);
    priv_showForm (eForm_main_syncWithCPU);

    isInterruptActive=false;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerInterrupt()));
    timer->start(50);
}


//********************************************************************************
MainWindow::~MainWindow()
{
    delete ui;
}

//*****************************************************
void MainWindow::priv_scheduleFormChange(eForm w)
{
    nextForm = w;
}

//*****************************************************
void MainWindow::priv_showForm (eForm w)
{
    this->hide();

    if (frmProg)
    {
        frmProg->hide();
        delete frmProg;
        frmProg = NULL;
    }

    if (frmBoot)
        frmBoot->hide();

    currentForm = w;

    switch (currentForm)
    {
    case eForm_main_syncWithCPU:
        ui->webView->setVisible(false);
        ui->labInfo->setVisible(true);
        this->ui->labInfo->setText(QString("Current folder: ") +rhea::getPhysicalPathToAppFolder());
        this->show();
        utils::hideMouse();
        stato = eStato_sync_1_queryIniParam;
        break;

    case eForm_boot:
        frmBoot->showMe();
        utils::hideMouse();
        break;

    case eForm_main_showBrowser:
        {
            ui->labInfo->setVisible(false);
            this->show();

            //carico la GUI nel browser
            retCode = 0;
            char s[1024];
            sprintf_s (s, sizeof(s), "%s/web/startup.html", glob->current_GUI);
            if (rhea::fs::fileExists(s))
                sprintf_s (s, sizeof(s), "file://%s/web/startup.html", glob->current_GUI);
            else
                sprintf_s (s, sizeof(s), "file://%s/varie/no-gui-installed.html", rhea::getPhysicalPathToAppFolder());

            ui->webView->setVisible(true);
            ui->webView->load(QUrl(s));
            utils::hideMouse();
            ui->webView->raise();
            ui->webView->setFocus();
        }
        break;

    case eForm_prog:
        frmProg = new FormProg(this, glob);
        frmProg->showMe();
        utils::hideMouse();
        break;
    }
}

//*****************************************************
void MainWindow::priv_addText (const char *s)
{
    //ui->labInfo->setText(ui->labInfo->text() +"\n" +QString(s));
    ui->labInfo->setText(s);
    utils::waitAndProcessEvent(300);
}


//*****************************************************
void MainWindow::timerInterrupt()
{
    if (isInterruptActive)
        return;
    isInterruptActive=true;


    if (nextForm != currentForm)
        priv_showForm(nextForm);

    switch (currentForm)
    {
    case eForm_main_syncWithCPU:
        priv_syncWithCPU_onTick();
        break;

    case eForm_boot:
        if (frmBoot->onTick() != 0)
            priv_scheduleFormChange(eForm_main_showBrowser);
        break;

    case eForm_main_showBrowser:
        if (priv_showBrowser_onTick() != 0)
            priv_scheduleFormChange(eForm_prog);
        break;

    case eForm_prog:
        if (frmProg->onTick() != 0)
            priv_scheduleFormChange(eForm_main_syncWithCPU);
        break;
    }

    isInterruptActive=false;
}


//*****************************************************
void MainWindow::priv_syncWithCPU_onTick()
{
    //vediamo se CPUBridge ha qualcosa da dirmi
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg(glob->subscriber.hFromCpuToOtherR, &msg))
    {
        priv_syncWithCPU_onCPUBridgeNotification(msg);
        rhea::thread::deleteMsg(msg);
    }

    //Aspetto di vedere la CPU viva e poi sincronizzo il mio DA3 con il suo
    switch (stato)
    {
    case eStato_sync_1_queryIniParam:
        priv_addText("Querying ini param...");
        cpubridge::ask_CPU_QUERY_INI_PARAM(glob->subscriber, 0);
        statoTimeout = rhea::getTimeNowMSec() + 10000;
        stato = eStato_sync_1_wait;
        break;

    case eStato_sync_1_wait:
        if (rhea::getTimeNowMSec() >= statoTimeout)
            stato = eStato_sync_1_queryIniParam;
        break;

    case eStato_sync_2_queryCpuStatus:
        priv_addText("Querying cpu status...");
        cpubridge::ask_CPU_QUERY_STATE(glob->subscriber, 0);
        statoTimeout = rhea::getTimeNowMSec() + 5000;
        stato = eStato_sync_2_wait;
        break;

    case eStato_sync_2_wait:
        if (rhea::getTimeNowMSec() >= statoTimeout)
            stato = eStato_sync_2_queryCpuStatus;
        break;

    case eStato_sync_3_queryVMCSettingTS:
        priv_addText("Querying cpu vmc settings timestamp...");
        cpubridge::ask_CPU_VMCDATAFILE_TIMESTAMP(glob->subscriber, 0);
        statoTimeout = rhea::getTimeNowMSec() + 10000;
        stato = eStato_sync_3_wait;
        break;

    case eStato_sync_3_wait:
        if (rhea::getTimeNowMSec() >= statoTimeout)
            stato = eStato_sync_2_queryCpuStatus;
        break;

    case eStato_sync_4_downloadVMCSetting:
        priv_addText("Downloading VMC Settings from CPU...");
        cpubridge::ask_READ_VMCDATAFILE(glob->subscriber, 0);
        statoTimeout = rhea::getTimeNowMSec() + 60000;
        stato = eStato_sync_4_wait;
        break;

    case eStato_sync_4_wait:
        if (rhea::getTimeNowMSec() >= statoTimeout)
            stato = eStato_sync_2_queryCpuStatus;
        break;

    case eStato_running:
        //Se c'è la chiavetta USB, andiamo in frmBoot, altrimenti direttamente in frmBrowser
        if (NULL != glob->usbFolder && rhea::fs::folderExists(glob->usbFolder))
            priv_scheduleFormChange (eForm_boot);
        else
            priv_scheduleFormChange (eForm_main_showBrowser);

    }
}

/**************************************************************************
 * priv_onCPUBridgeNotification
 *
 * E' arrivato un messaggio da parte di CPUBrdige sulla msgQ dedicata (ottenuta durante la subscribe di this a CPUBridge).
 */
void MainWindow::priv_syncWithCPU_onCPUBridgeNotification (rhea::thread::sMsg &msg)
{
    const u16 handlerID = (msg.paramU32 & 0x0000FFFF);
    assert (handlerID == 0);

    const u16 notifyID = (u16)msg.what;
    switch (notifyID)
    {
    case CPUBRIDGE_NOTIFY_CPU_INI_PARAM:
        {
            cpubridge::sCPUParamIniziali iniParam;
            cpubridge::translateNotify_CPU_INI_PARAM (msg, &iniParam);
            strcpy (glob->cpuVersion, iniParam.CPU_version);
            stato = eStato_sync_2_queryCpuStatus;
        }
        break;

    case CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED:
        {
            cpubridge::eVMCState vmcState;
            u8 vmcErrorCode, vmcErrorType;
            cpubridge::translateNotify_CPU_STATE_CHANGED (msg, &vmcState, &vmcErrorCode, &vmcErrorType);
            if (vmcState != cpubridge::eVMCState_COM_ERROR)
                stato = eStato_sync_3_queryVMCSettingTS;
            else
                stato = eStato_running;
        }
        break;

    case CPUBRIDGE_NOTIFY_VMCDATAFILE_TIMESTAMP:
        {
            cpubridge::sCPUVMCDataFileTimeStamp cpuTS;
            cpubridge::translateNotify_CPU_VMCDATAFILE_TIMESTAMP (msg, &cpuTS);

            //nel caso in cui sono connesso alla finta CPU software...
            if (strcasecmp(glob->cpuVersion, "FAKE CPU") == 0)
            {
                stato = eStato_running;
                break;
            }

            //vediamo se il TS della CPU è lo stesso mio
            cpubridge::loadVMCDataFileTimeStamp(&myTS);

            if (myTS == cpuTS)
                stato = eStato_running;
            else
            {
                myTS = cpuTS;
                rhea::fs::deleteAllFileInFolderRecursively (glob->last_installed_da3, false);
                stato = eStato_sync_4_downloadVMCSetting;
            }
            break;
        }
        break;

    case CPUBRIDGE_NOTIFY_READ_VMCDATAFILE_PROGRESS:
        {
            cpubridge::eReadDataFileStatus status;
            u16 totKbSoFar = 0;
            u16 fileID = 0;
            cpubridge::translateNotify_READ_VMCDATAFILE_PROGRESS (msg, &status, &totKbSoFar, &fileID);

            char s[512];
            if (status == cpubridge::eReadDataFileStatus_inProgress)
            {
                sprintf_s (s, sizeof(s), "Downloading VMC Settings... %d Kb", totKbSoFar);
                priv_addText (s);
            }
            else if (status == cpubridge::eReadDataFileStatus_finishedOK)
            {
                sprintf_s (s, sizeof(s), "Downloading VMC Settings... SUCCESS");
                priv_addText (s);

                //copio il file appena scricato dalla CPU nell mie cartelle locali
                char src[256];
                sprintf_s (src, sizeof(src), "%s/vmcDataFile%d.da3", glob->tempFolder, fileID);

                char dst[256];
                sprintf_s (dst, sizeof(dst), "%s/vmcDataFile.da3", glob->current_da3);
                if (rhea::fs::fileExists(dst))
                    rhea::fs::fileDelete(dst);
                rhea::fs::fileCopy(src, dst);

                cpubridge::saveVMCDataFileTimeStamp(myTS);


                //copio anche in cartella last_installed
                rhea::DateTime dt;
                dt.setNow();
                dt.formatAs_YYYYMMDDHHMMSS(s, sizeof(s), '-', '-', '_');
                sprintf_s (dst, sizeof(dst), "%s/download_from_cpu_%s.da3", glob->last_installed_da3, s);

                rhea::fs::fileCopy(src, dst);

                stato = eStato_sync_3_queryVMCSettingTS;
            }
            else
            {
                sprintf_s (s, sizeof(s), "Downloading VMC Settings... ERROR: %s", rhea::app::utils::verbose_readDataFileStatus(status));
                priv_addText(s);
                stato = eStato_sync_2_queryCpuStatus;
                utils::waitAndProcessEvent(3000);
            }

        }
        break;
    }
}





//********************************************************************************
int MainWindow::priv_showBrowser_onTick()
{
    if (retCode != 0)
        return retCode;

    //vediamo se CPUBridge ha qualcosa da dirmi
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg(glob->subscriber.hFromCpuToOtherR, &msg))
    {
        priv_showBrowser_onCPUBridgeNotification(msg);
        rhea::thread::deleteMsg(msg);
    }

    return 0;
}

//********************************************************************************
void MainWindow::priv_showBrowser_onCPUBridgeNotification (rhea::thread::sMsg &msg)
{
    const u16 handlerID = (msg.paramU32 & 0x0000FFFF);
    assert (handlerID == 0);

    const u16 notifyID = (u16)msg.what;
    switch (notifyID)
    {
    case CPUBRIDGE_NOTIFY_CPU_INI_PARAM:
        {
            cpubridge::sCPUParamIniziali iniParam;
            cpubridge::translateNotify_CPU_INI_PARAM (msg, &iniParam);
            strcpy (glob->cpuVersion, iniParam.CPU_version);
        }
        break;

    case CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS:
        {
            cpubridge::eRunningSelStatus s = cpubridge::eRunningSelStatus_finished_KO;
            cpubridge::translateNotify_CPU_RUNNING_SEL_STATUS (msg, &s);
            if (s == cpubridge::eRunningSelStatus_finished_OK)
                History::incCounterSelezioni();
        }
        break;

    case CPUBRIDGE_NOTIFY_BTN_PROG_PRESSED:
        //l'utente ha premuto il btn PROG, devo andare in programmazione
        retCode = 1;
        break;
    }
}
