#include "header.h"
#include <QTimer>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../rheaAppLib/rheaAppUtils.h"



//********************************************************************************
MainWindow::MainWindow (sGlobal *globIN) :
        QMainWindow(NULL),
        ui(new Ui::MainWindow)
{
    glob = globIN;

    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    utils::hideMouse();

    frmBoot = NULL;
    if (NULL != glob->usbFolder)
    {
        frmBoot = new FormBoot(this, glob);
        frmBoot->hide();
    }

    frmBrowser = new FormBrowser(this, glob);
    frmBrowser->hide();

    frmProg = NULL;

    priv_scheduleFormChange (eForm_main);
    priv_showForm(eForm_main);

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
    currentForm = w;
    switch (currentForm)
    {
    case eForm_main:
        if (frmProg)
        {
            frmProg->hide();
            delete frmProg;
            frmProg = NULL;
        }
        this->show();
        stato = eStato_sync_1_queryIniParam;
        break;

    case eForm_boot:
        this->hide();
        frmBoot->showMe();
        break;

    case eForm_browser:
        if (frmBoot)
            frmBoot->hide();
        else
            this->hide();
        frmBrowser->showMe();
        break;

    case eForm_prog:
        frmBrowser->hide();
        frmProg = new FormProg(this, glob);
        frmProg->showMe();
        break;
    }
}

//*****************************************************
void MainWindow::priv_setText (const char *s)
{
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
    case eForm_main:
        priv_onTick();
        break;

    case eForm_boot:
        if (frmBoot->onTick() != 0)
            priv_scheduleFormChange(eForm_browser);
        break;

    case eForm_browser:
        if (frmBrowser->onTick() != 0)
            priv_scheduleFormChange(eForm_prog);
        break;

    case eForm_prog:
        if (frmProg->onTick() != 0)
            priv_scheduleFormChange(eForm_main);
        break;
    }

    isInterruptActive=false;
}


//*****************************************************
void MainWindow::priv_onTick()
{
    //vediamo se CPUBridge ha qualcosa da dirmi
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg(glob->subscriber.hFromCpuToOtherR, &msg))
    {
        priv_onCPUBridgeNotification(msg);
        rhea::thread::deleteMsg(msg);
    }

    //Aspetto di vedere la CPU viva e poi sincronizzo il mio DA3 con il suo
    switch (stato)
    {
    case eStato_sync_1_queryIniParam:
        priv_setText("Querying ini param...");
        cpubridge::ask_CPU_QUERY_INI_PARAM(glob->subscriber, 0);
        statoTimeout = rhea::getTimeNowMSec() + 10000;
        stato = eStato_sync_1_wait;
        break;

    case eStato_sync_1_wait:
        if (rhea::getTimeNowMSec() >= statoTimeout)
            stato = eStato_sync_1_queryIniParam;
        break;

    case eStato_sync_2_queryCpuStatus:
        priv_setText("Querying cpu status...");
        cpubridge::ask_CPU_QUERY_STATE(glob->subscriber, 0);
        statoTimeout = rhea::getTimeNowMSec() + 5000;
        stato = eStato_sync_2_wait;
        break;

    case eStato_sync_2_wait:
        if (rhea::getTimeNowMSec() >= statoTimeout)
            stato = eStato_sync_2_queryCpuStatus;
        break;

    case eStato_sync_3_queryVMCSettingTS:
        priv_setText("Querying cpu vmc settings timestamp...");
        cpubridge::ask_CPU_VMCDATAFILE_TIMESTAMP(glob->subscriber, 0);
        statoTimeout = rhea::getTimeNowMSec() + 10000;
        stato = eStato_sync_3_wait;
        break;

    case eStato_sync_3_wait:
        if (rhea::getTimeNowMSec() >= statoTimeout)
            stato = eStato_sync_2_queryCpuStatus;
        break;

    case eStato_sync_4_downloadVMCSetting:
        priv_setText("Downloading VMC Settings from CPU...");
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
            priv_scheduleFormChange (eForm_browser);

    }
}

/**************************************************************************
 * priv_onCPUBridgeNotification
 *
 * E' arrivato un messaggio da parte di CPUBrdige sulla msgQ dedicata (ottenuta durante la subscribe di this a CPUBridge).
 */
void MainWindow::priv_onCPUBridgeNotification (rhea::thread::sMsg &msg)
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
                priv_setText (s);
            }
            else if (status == cpubridge::eReadDataFileStatus_finishedOK)
            {
                sprintf_s (s, sizeof(s), "Downloading VMC Settings... SUCCESS");
                priv_setText (s);

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
                priv_setText(s);
                stato = eStato_sync_2_queryCpuStatus;
                utils::waitAndProcessEvent(3000);
            }

        }
        break;
    }
}


