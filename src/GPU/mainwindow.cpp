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
#ifdef _DEBUG
    setWindowFlags(Qt::Window);
#else
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    utils::hideMouse();
#endif

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
        cpubridge::ask_CPU_QUERY_STATE(glob->subscriber, 0);
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

    case eForm_newprog_lavaggioMilker:
        {
            char s[256];
            sprintf_s (s, sizeof(s), "file://%s/varie/prog/index.html?page=pageCleaningMilker", rhea::getPhysicalPathToAppFolder());
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
            case eRetCode_gotoNewMenuProg_lavaggioMilker: priv_scheduleFormChange(eForm_newprog_lavaggioMilker); break;
        }
        break;

    case eForm_specialActionBeforeGUI:
        switch (frmPreGUI->onTick())
        {
            default: break;
            case eRetCode_gotoFormBrowser: priv_scheduleFormChange(eForm_main_showBrowser); break;
            case eRetCode_gotoFormOldMenuProg: priv_scheduleFormChange(eForm_oldprog_legacy); break;
            case eRetCode_gotoNewMenuProg_LavaggioSanitario: priv_scheduleFormChange(eForm_newprog_lavaggioSanitario); break;
            case eRetCode_gotoNewMenuProg_lavaggioMilker: priv_scheduleFormChange(eForm_newprog_lavaggioMilker); break;
        }
        break;

    case eForm_main_showBrowser:
        switch (priv_showBrowser_onTick())
        {
            default: break;
            case eRetCode_gotoFormOldMenuProg: priv_scheduleFormChange(eForm_oldprog_legacy); break;
            case eRetCode_gotoNewMenuProgrammazione: priv_scheduleFormChange(eForm_newprog); break;
            case eRetCode_gotoNewMenuProg_LavaggioSanitario: priv_scheduleFormChange(eForm_newprog_lavaggioSanitario); break;
            case eRetCode_gotoNewMenuProg_lavaggioMilker: priv_scheduleFormChange(eForm_newprog_lavaggioMilker); break;
        }
        break;

    case eForm_oldprog_legacy:
        if (frmProg->onTick() != eRetCode_none)
            priv_scheduleFormChange(eForm_main_syncWithCPU);
        break;

    case eForm_newprog:
    case eForm_newprog_lavaggioMilker:
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
    //rimango in questo stato fino a che la CPU non ha finito di fare le sue cose.
    //La CPU ha finito quando il suo stato diventa diverso da uno dei 2 seguenti:
    //  eVMCState_COMPATIBILITY_CHECK
    //  eVMCState_DA3_SYNC
    //
    //vediamo se CPUBridge ha qualcosa da dirmi
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg(glob->subscriber.hFromCpuToOtherR, &msg))
    {
        priv_syncWithCPU_onCPUBridgeNotification(msg);
        rhea::thread::deleteMsg(msg);
    }

    if (syncWithCPU.stato == 0)
    {
        if (syncWithCPU.vmcState != cpubridge::eVMCState_COMPATIBILITY_CHECK && syncWithCPU.vmcState != cpubridge::eVMCState_DA3_SYNC)
        {
            if (syncWithCPU.vmcState == cpubridge::eVMCState_CPU_NOT_SUPPORTED)
            {
                //CPU non supportata, andiamo direttamente in form boot dove mostriamo il msg di errore e chiediamo di uppare un nuovo FW
                this->glob->bSyncWithCPUResult = false;
                priv_scheduleFormChange (eForm_boot);
                return;
            }


            //Se siamo in com_error, probabilmente non c'era nemmeno un FW CPU in gradi di rispondere, per cui funziona come sopra
            if (syncWithCPU.vmcState == cpubridge::eVMCState_COM_ERROR)
            {
                //CPU non supportata, andiamo direttamente in form boot dove mostriamo il msg di errore e chiediamo di uppare un nuovo FW
                this->glob->bSyncWithCPUResult = false;
                priv_scheduleFormChange (eForm_boot);
                return;
            }

            //Ok, pare che tutto sia in ordine
            this->glob->bSyncWithCPUResult = true;

            //Prima di proseguire chiedo un po' di parametri di configurazione
            syncWithCPU.stato = 1;
            cpubridge::ask_CPU_QUERY_INI_PARAM(glob->subscriber, 0);
        }

        /*periodicamente chiedo lo stato della CPU per visualizzarlo a video
        const u64 timeNowMSec = rhea::getTimeNowMSec();
        if (timeNowMSec > syncWithCPU.nextTimeoutAskCPUStateMSec)
        {
            syncWithCPU.nextTimeoutAskCPUStateMSec = timeNowMSec + 1000;
            cpubridge::ask_CPU_QUERY_STATE(glob->subscriber, 0);
        }*/
    }
    else
    {
        if (syncWithCPU.stato == 3)
        {
            //abbiamo tutte le info, possiamo partire
            syncWithCPU.stato = 4;

            //Se c'è la chiavetta USB, andiamo in frmBoot, altrimenti direttamente in frmBrowser
             if (rhea::fs::folderExists(glob->usbFolder))
                 priv_scheduleFormChange (eForm_boot);
             else
                 priv_scheduleFormChange (eForm_specialActionBeforeGUI);
             return;
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
            if (syncWithCPU.stato == 1)
            {
                syncWithCPU.stato = 2;
                cpubridge::ask_CPU_GET_EXTENDED_CONFIG_INFO(glob->subscriber, 0);
            }
        }
        break;

    case CPUBRIDGE_NOTIFY_EXTENDED_CONFIG_INFO:
        {
            cpubridge::translateNotify_EXTENDED_CONFIG_INFO(msg, &glob->extendedCPUInfo);
            if (syncWithCPU.stato == 2)
                syncWithCPU.stato = 3;
        }
        break;

    case CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED:
        {
            u8 vmcErrorCode=0, vmcErrorType=0;
            u16 flag1=0;
            cpubridge::translateNotify_CPU_STATE_CHANGED (msg, &syncWithCPU.vmcState, &vmcErrorCode, &vmcErrorType, &flag1);
            priv_addText (rhea::app::utils::verbose_eVMCState(syncWithCPU.vmcState));
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
            }
            else
            {
                sprintf_s (s, sizeof(s), "Downloading VMC Settings... ERROR: %s", rhea::app::utils::verbose_readDataFileStatus(status));
                priv_addText(s);
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
            u8 vmcErrorCode=0, vmcErrorType=0;
            u16 flag1=0;
            cpubridge::translateNotify_CPU_STATE_CHANGED (msg, &vmcState, &vmcErrorCode, &vmcErrorType, &flag1);

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
    if (currentForm < eForm_newprog)
        return;

    QString url = arg1.toString();
    if (url.indexOf("gotoLegacyMenu.html") > 0)
    {
        //dal nuovo menu di programmazione, vogliamo andare in quello vecchio!
        retCode = eRetCode_gotoFormOldMenuProg;
    }
    else if (url.indexOf("gotoHMI.html") > 0)
    {
        //dal nuovo menu di programmazione, vogliamo andare in quello vecchio!
        retCode = eRetCode_gotoFormBrowser;
    }
}
