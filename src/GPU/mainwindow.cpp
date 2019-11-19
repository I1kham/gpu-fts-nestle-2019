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
    retCode = eRetCode_none;
    frmPreGUI = NULL;
    syncWithCPU.reset();

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
void MainWindow::priv_loadURL (const char *url)
{
    ui->labInfo->setVisible(false);
    this->show();

    //carico la GUI nel browser
    retCode = eRetCode_none;
    ui->webView->setVisible(true);
    ui->webView->load(QUrl(url));
    utils::hideMouse();
    ui->webView->raise();
    ui->webView->setFocus();
}


//*****************************************************
bool MainWindow::priv_shouldIShowFormPreGUI()
{
    char s[256];
    sprintf_s (s, sizeof(s), "%s/vmcDataFile.da3", glob->current_da3);
    DA3 *da3 = new DA3();
    da3->loadInMemory (rhea::memory_getDefaultAllocator(), s, glob->extendedCPUInfo.machineType, glob->extendedCPUInfo.machineModel);

    u16 groundCounterLimit = da3->getDecounterCoffeeGround();
    bool bShowBtnResetGroundConter = (groundCounterLimit > 0);
    bool bShowBtnCleanMilker = false;
    if (!da3->isInstant() && da3->getMilker_RinseUserMsg() > 0)
        bShowBtnCleanMilker = true;

    delete da3;

    if (!bShowBtnResetGroundConter && !bShowBtnCleanMilker)
        return false;

    if (NULL == frmPreGUI)
        frmPreGUI = new FormPreGui(this, glob);
    frmPreGUI->showMe(groundCounterLimit, bShowBtnCleanMilker);
    return true;
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

    if (frmPreGUI)
        frmPreGUI->hide();

    currentForm = w;

    switch (currentForm)
    {
    case eForm_main_syncWithCPU:
        syncWithCPU.reset();
        glob->bSyncWithCPUResult = false;
        ui->webView->setVisible(false);
        ui->labInfo->setVisible(true);
        this->ui->labInfo->setText("");
        //this->ui->labInfo->setText(QString("Current folder: ") +rhea::getPhysicalPathToAppFolder());
        this->show();
        utils::hideMouse();

        break;

    case eForm_boot:
        frmBoot->showMe();
        utils::hideMouse();
        break;

    case eForm_specialActionBeforeGUI:
        if (!priv_shouldIShowFormPreGUI())
        {
            priv_scheduleFormChange(eForm_main_showBrowser);
            priv_showForm(eForm_main_showBrowser);
        }
        break;

    case eForm_main_showBrowser:
        {
            char s[1024];
            sprintf_s (s, sizeof(s), "%s/web/startup.html", glob->current_GUI);
            if (rhea::fs::fileExists(s))
                sprintf_s (s, sizeof(s), "file://%s/web/startup.html", glob->current_GUI);
            else
                sprintf_s (s, sizeof(s), "file://%s/varie/no-gui-installed.html", rhea::getPhysicalPathToAppFolder());

            priv_loadURL(s);
            cpubridge::ask_CPU_QUERY_INI_PARAM(glob->subscriber, 0);
        }
        break;

    case eForm_oldprog_legacy:
        frmProg = new FormProg(this, glob);
        frmProg->showMe();
        utils::hideMouse();
        break;

    case eForm_newprog:
        {
            char s[256];
            sprintf_s (s, sizeof(s), "file://%s/varie/prog/index.html", rhea::getPhysicalPathToAppFolder());
            priv_loadURL(s);
        }
        break;

    case eForm_newprog_lavaggioSanitario:
        {
            char s[256];
            sprintf_s (s, sizeof(s), "file://%s/varie/prog/index.html?page=pageCleaningSanitario", rhea::getPhysicalPathToAppFolder());
            priv_loadURL(s);
        }
        break;
    }
}

//*****************************************************
void MainWindow::priv_addText (const char *s)
{
    ui->labInfo->setText(ui->labInfo->text() +"\n" +QString(s));
    //ui->labInfo->setText(s);
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
        switch (frmBoot->onTick())
        {
            default: break;
            case eRetCode_gotoFormBrowser: priv_scheduleFormChange(eForm_specialActionBeforeGUI); break;
            case eRetCode_gotoFormOldMenuProg: priv_scheduleFormChange(eForm_oldprog_legacy); break;
            case eRetCode_gotoNewMenuProg_LavaggioSanitario: priv_scheduleFormChange(eForm_newprog_lavaggioSanitario); break;
        }
        break;

    case eForm_specialActionBeforeGUI:
        switch (frmPreGUI->onTick())
        {
            default: break;
            case eRetCode_gotoFormBrowser: priv_scheduleFormChange(eForm_main_showBrowser); break;
            case eRetCode_gotoFormOldMenuProg: priv_scheduleFormChange(eForm_oldprog_legacy); break;
            case eRetCode_gotoNewMenuProg_LavaggioSanitario: priv_scheduleFormChange(eForm_newprog_lavaggioSanitario); break;
        }
        break;

    case eForm_main_showBrowser:
        switch (priv_showBrowser_onTick())
        {
            default: break;
            case eRetCode_gotoFormOldMenuProg: priv_scheduleFormChange(eForm_oldprog_legacy); break;
            case eRetCode_gotoNewMenuProgrammazione: priv_scheduleFormChange(eForm_newprog); break;
            case eRetCode_gotoNewMenuProg_LavaggioSanitario: priv_scheduleFormChange(eForm_newprog_lavaggioSanitario); break;
        }
        break;

    case eForm_oldprog_legacy:
        if (frmProg->onTick() != eRetCode_none)
            priv_scheduleFormChange(eForm_main_syncWithCPU);
        break;

    case eForm_newprog:
    case eForm_newprog_lavaggioSanitario:
        switch (priv_showNewProgrammazione_onTick())
        {
            default: break;
            case eRetCode_gotoFormBrowser: priv_scheduleFormChange(eForm_main_showBrowser); break;
            case eRetCode_gotoFormOldMenuProg: priv_scheduleFormChange(eForm_oldprog_legacy); break;
        }
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
    switch (syncWithCPU.stato)
    {
    case eStato_sync_1_queryIniParam:
        priv_addText("Querying ini param...");
        cpubridge::ask_CPU_QUERY_INI_PARAM(glob->subscriber, 0);
        syncWithCPU.timeoutMSec = rhea::getTimeNowMSec() + 2000;
        syncWithCPU.stato = eStato_sync_1_wait;
        break;

    case eStato_sync_1_wait:
        if (rhea::getTimeNowMSec() >= syncWithCPU.timeoutMSec)
        {
            syncWithCPU.nRetryLeft--;
            if (syncWithCPU.nRetryLeft == 0)
                syncWithCPU.stato = eStato_running;
            else
                syncWithCPU.stato = eStato_sync_1_queryIniParam;
        }
        break;

    case eStato_sync_2_queryExtendedConfigInfo:
        priv_addText("Querying extended cpu info...");
        cpubridge::ask_CPU_GET_EXTENDED_CONFIG_INFO(glob->subscriber, 0);
        syncWithCPU.timeoutMSec = rhea::getTimeNowMSec() + 2000;
        syncWithCPU.stato = eStato_sync_2_wait;
        break;

    case eStato_sync_2_wait:
        if (rhea::getTimeNowMSec() >= syncWithCPU.timeoutMSec)
        {
            syncWithCPU.nRetryLeft--;
            if (syncWithCPU.nRetryLeft == 0)
                syncWithCPU.stato = eStato_running;
            else
                syncWithCPU.stato = eStato_sync_2_queryExtendedConfigInfo;
        }
        break;

    case eStato_sync_3_queryCpuStatus:
        priv_addText("Querying cpu status...");
        cpubridge::ask_CPU_QUERY_STATE(glob->subscriber, 0);
        syncWithCPU.timeoutMSec = rhea::getTimeNowMSec() + 2000;
        syncWithCPU.stato = eStato_sync_3_wait;
        break;

    case eStato_sync_3_wait:
        if (rhea::getTimeNowMSec() >= syncWithCPU.timeoutMSec)
        {
            syncWithCPU.nRetryLeft--;
            if (syncWithCPU.nRetryLeft == 0)
                syncWithCPU.stato = eStato_running;
            else
                syncWithCPU.stato = eStato_sync_3_queryCpuStatus;
        }
        break;

    case eStato_sync_4_queryVMCSettingTS:
        priv_addText("Querying cpu vmc settings timestamp...");
        cpubridge::ask_CPU_VMCDATAFILE_TIMESTAMP(glob->subscriber, 0);
        syncWithCPU.timeoutMSec = rhea::getTimeNowMSec() + 10000;
        syncWithCPU.stato = eStato_sync_4_wait;
        break;

    case eStato_sync_4_wait:
        if (rhea::getTimeNowMSec() >= syncWithCPU.timeoutMSec)
        {
            syncWithCPU.nRetryLeft--;
            if (syncWithCPU.nRetryLeft == 0)
                syncWithCPU.stato = eStato_running;
            else
                syncWithCPU.stato = eStato_sync_4_queryVMCSettingTS;
        }
        break;

    case eStato_sync_5_downloadVMCSetting:
        priv_addText("Downloading VMC Settings from CPU...");
        cpubridge::ask_READ_VMCDATAFILE(glob->subscriber, 0);
        syncWithCPU.timeoutMSec = rhea::getTimeNowMSec() + 25000;
        syncWithCPU.stato = eStato_sync_5_wait;
        break;

    case eStato_sync_5_wait:
        if (rhea::getTimeNowMSec() >= syncWithCPU.timeoutMSec)
        {
            syncWithCPU.nRetryLeft--;
            if (syncWithCPU.nRetryLeft == 0)
                syncWithCPU.stato = eStato_running;
            else
                syncWithCPU.stato = eStato_sync_3_queryCpuStatus;
        }
        break;

    case eStato_running:
        //Se c'è la chiavetta USB, andiamo in frmBoot, altrimenti direttamente in frmBrowser
        if (false == glob->bSyncWithCPUResult)
            priv_scheduleFormChange (eForm_boot);
        else
        {
            if (rhea::fs::folderExists(glob->usbFolder))
                priv_scheduleFormChange (eForm_boot);
            else
                priv_scheduleFormChange (eForm_specialActionBeforeGUI);
        }
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

            if (syncWithCPU.stato == eStato_sync_1_wait)
            {
                syncWithCPU.stato = eStato_sync_2_queryExtendedConfigInfo;
                syncWithCPU.nRetryLeft = 3;
            }
        }
        break;

    case CPUBRIDGE_NOTIFY_EXTENDED_CONFIG_INFO:
        {
            if (syncWithCPU.stato == eStato_sync_2_wait)
            {
                cpubridge::translateNotify_EXTENDED_CONFIG_INFO(msg, &glob->extendedCPUInfo);
                syncWithCPU.stato = eStato_sync_3_queryCpuStatus;
                syncWithCPU.nRetryLeft = 3;
            }
        }
        break;

    case CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED:
        {
            cpubridge::eVMCState vmcState;
            u8 vmcErrorCode, vmcErrorType;
            cpubridge::translateNotify_CPU_STATE_CHANGED (msg, &vmcState, &vmcErrorCode, &vmcErrorType);
            if (syncWithCPU.stato == eStato_sync_3_wait)
            {
                if (vmcState != cpubridge::eVMCState_COM_ERROR)
                {
                    syncWithCPU.stato = eStato_sync_4_queryVMCSettingTS;
                    syncWithCPU.nRetryLeft = 3;
                }
                else
                    syncWithCPU.stato = eStato_running;
            }
        }
        break;

    case CPUBRIDGE_NOTIFY_VMCDATAFILE_TIMESTAMP:
        {
            if (syncWithCPU.stato == eStato_sync_4_wait)
            {
                cpubridge::sCPUVMCDataFileTimeStamp cpuTS;
                cpubridge::translateNotify_CPU_VMCDATAFILE_TIMESTAMP (msg, &cpuTS);

                //nel caso in cui sono connesso alla finta CPU software...
                if (strcasecmp(glob->cpuVersion, "FAKE CPU") == 0)
                {
                    this->glob->bSyncWithCPUResult = true;
                    syncWithCPU.stato = eStato_running;
                    syncWithCPU.nRetryLeft = 3;
                    break;
                }

                //vediamo se il TS della CPU è lo stesso mio
                cpubridge::loadVMCDataFileTimeStamp(&myTS);

                if (myTS == cpuTS)
                {
                    this->glob->bSyncWithCPUResult = true;
                    syncWithCPU.stato = eStato_running;
                }
                else
                {
                    myTS = cpuTS;
                    syncWithCPU.stato = eStato_sync_5_downloadVMCSetting;
                    syncWithCPU.nRetryLeft = 3;
                }
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

                //copio il file appena scaricato dalla CPU nella mia cartelle locale current/da3
                char src[256];
                sprintf_s (src, sizeof(src), "%s/vmcDataFile%d.da3", glob->tempFolder, fileID);

                char dst[256];
                sprintf_s (dst, sizeof(dst), "%s/vmcDataFile.da3", glob->current_da3);
                if (rhea::fs::fileExists(dst))
                    rhea::fs::fileDelete(dst);
                rhea::fs::fileCopy(src, dst);

                cpubridge::saveVMCDataFileTimeStamp(myTS);


                //Se nella cartella last_installed non c'è nulla, allora il da3 scaricato dalla CPU
                //lo chiamo "downloaded_from_cpu.da3", altrimenti mantengo il nome del file precedente
                sprintf_s (dst, sizeof(dst), "download_from_cpu.da3");
                OSFileFind ff;
                if (rhea::fs::findFirst (&ff, glob->last_installed_da3, "*.da3"))
                {
                    do
                    {
                        if (!rhea::fs::findIsDirectory(ff))
                        {
                            sprintf_s(dst, sizeof(dst), "%s", rhea::fs::findGetFileName(ff));
                            break;
                        }
                    } while (rhea::fs::findNext(ff));
                    rhea::fs::findClose(ff);
                }
                rhea::fs::deleteAllFileInFolderRecursively (glob->last_installed_da3, false);

                //copio anche in cartella last_installed
                sprintf_s (s, sizeof(s), "%s/%s", glob->last_installed_da3, dst);
                rhea::fs::fileCopy(src, s);

                //aggiorno il file con data e ora di ultima modifica
                rhea::DateTime dt;
                dt.setNow();
                sprintf_s(s, sizeof(s), "%s/last_installed/da3/dateUM.bin", rhea::getPhysicalPathToAppFolder());
                FILE *f = fopen(s, "wb");
                u64 u = dt.getInternalRappresentation();
                fwrite (&u, sizeof(u64), 1, f);
                fclose(f);

                syncWithCPU.stato = eStato_sync_4_queryVMCSettingTS;
            }
            else
            {
                sprintf_s (s, sizeof(s), "Downloading VMC Settings... ERROR: %s", rhea::app::utils::verbose_readDataFileStatus(status));
                priv_addText(s);
                syncWithCPU.stato = eStato_sync_3_queryCpuStatus;
                utils::waitAndProcessEvent(3000);
            }

        }
        break;
    }
}





//********************************************************************************
eRetCode MainWindow::priv_showBrowser_onTick()
{
    if (retCode != eRetCode_none)
        return retCode;

    //vediamo se CPUBridge ha qualcosa da dirmi
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg(glob->subscriber.hFromCpuToOtherR, &msg))
    {
        priv_showBrowser_onCPUBridgeNotification(msg);
        rhea::thread::deleteMsg(msg);
    }

    return eRetCode_none;
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

    case CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED:
        {
            cpubridge::eVMCState vmcState;
            u8 vmcErrorCode, vmcErrorType;
            cpubridge::translateNotify_CPU_STATE_CHANGED (msg, &vmcState, &vmcErrorCode, &vmcErrorType);

            //non dovrebbe mai succede che la CPU vada da sola in PROG, ma se succede io faccio apparire il vecchio menu PROG
            if (vmcState == cpubridge::eVMCState_PROGRAMMAZIONE)
                retCode = eRetCode_gotoFormOldMenuProg;
            //questo è il caso in cui la CPU non ha portato a termine un LAV SANITARIO. Spegnendo e riaccendendo la macchina, la
            //CPU va da sola in LAV_SANITARIO e io di conseguenza devo andare nel nuovo menu prog alla pagina corretta
            else if (vmcState == cpubridge::eVMCState_LAVAGGIO_SANITARIO)
                retCode = eRetCode_gotoNewMenuProg_LavaggioSanitario;
        }
        break;

    case CPUBRIDGE_NOTIFY_BTN_PROG_PRESSED:
        //l'utente ha premuto il btn PROG
#ifdef BTN_PROG_VA_IN_VECCHIO_MENU_PROGRAMMAZIONE
        //devo andare nel vecchio menu prog
        retCode = eRetCode_gotoFormProg;
#else
        retCode = eRetCode_gotoNewMenuProgrammazione;
#endif
        break;
    }
}

//********************************************************************************
eRetCode MainWindow::priv_showNewProgrammazione_onTick()
{
    if (retCode != eRetCode_none)
        return retCode;

    //vediamo se CPUBridge ha qualcosa da dirmi
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg(glob->subscriber.hFromCpuToOtherR, &msg))
    {
        priv_showNewProgrammazione_onCPUBridgeNotification(msg);
        rhea::thread::deleteMsg(msg);
    }

    return eRetCode_none;
}

//********************************************************************************
void MainWindow::priv_showNewProgrammazione_onCPUBridgeNotification (rhea::thread::sMsg &msg)
{
    const u16 handlerID = (msg.paramU32 & 0x0000FFFF);
    assert (handlerID == 0);

    const u16 notifyID = (u16)msg.what;
    switch (notifyID)
    {
    case CPUBRIDGE_NOTIFY_BTN_PROG_PRESSED:
        //l'utente ha premuto il btn PROG
        retCode = eRetCode_gotoFormBrowser;
        break;
    }
}

//********************************************************************************
void MainWindow::on_webView_urlChanged(const QUrl &arg1)
{
    if (currentForm != eForm_newprog)
        return;

    QString url = arg1.toString();
    if (url.indexOf("gotoLegacyMenu.html") > 0)
    {
        //dal nuovo menu di programmazione, vogliamo andare in quello vecchio!
        retCode = eRetCode_gotoFormOldMenuProg;
    }
}
