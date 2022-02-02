#include "header.h"
#include <QTimer>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "history.h"
#include "../rheaAppLib/rheaAppUtils.h"
#include "../rheaExternalSerialAPI/ESAPI.h"



//********************************************************************************
MainWindow::MainWindow (sGlobal *globIN) :
        QMainWindow(NULL),
        ui(new Ui::MainWindow)
{
    glob = globIN;
    retCode = eRetCode_none;
    frmPreGUI = NULL;
    nextTimeAskForCPULockStatus_msec = 0;
    syncWithCPU.reset();

    ui->setupUi(this);
    priv_showLockedPanel(false);

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
bool MainWindow::priv_autoupdate_exists() const
{
    if (NULL == frmBoot)
        return false;
    return frmBoot->does_autoupdate_exists();
}

//*****************************************************
void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    //simula (+ o -) pressione del btn PROG
    if (ev->key() == Qt::Key_P)
    {
        switch (currentForm)
        {
        default:
            break;

        case eForm_main_showBrowser:
            priv_scheduleFormChange(eForm_newprog);
            break;

        case eForm_oldprog_legacy:
        case eForm_newprog:
            priv_scheduleFormChange(eForm_main_showBrowser);
            break;
        }
    }
}

//*****************************************************
void MainWindow::priv_showLockedPanel (bool b)
{
    if (!b)
        ui->panelLocked->setVisible(false);
    else
    {
        ui->panelLocked->move (10, 10);
        ui->panelLocked->setVisible(true);
        ui->panelLocked->raise();
    }
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
void MainWindow::priv_loadURLMenuProg (const char *paramsInGet)
{
    char folder[256];
    sprintf_s (folder, sizeof(folder), "%s/varie/prog", rhea::getPhysicalPathToAppFolder());

    char lang[4];
    sprintf_s (lang, sizeof(lang),"GB");

    //se esiste il file lastUsedLang.txt, allora dentro c'è l'ultima lingua usata per il menu di prog
    char s[256];
    sprintf_s (s, sizeof(s), "%s/lastUsedLang.txt", folder);
    if (rhea::fs::fileExists((const u8*)s))
    {
        FILE *f = fopen(s,"rt");
        fread (lang, 2, 1, f);
        lang[2] = 0;
        rhea::fs::fileClose(f);
    }

    sprintf_s (s, sizeof(s), "%s/index_%s.html", folder, lang);
    if (!rhea::fs::fileExists((const u8*)s))
        sprintf_s (s, sizeof(s), "%s/index_GB.html", folder);

    if (NULL != paramsInGet)
    {
        strcat_s (s, sizeof(s), "?");
        strcat_s (s, sizeof(s), paramsInGet);
    }

    sprintf_s (folder, sizeof(folder), "file://%s", s);
    priv_loadURL(folder);
}

//*****************************************************
bool MainWindow::priv_shouldIShowFormPreGUI()
{
    return false;
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
        syncWithCPU.nextTimeoutAskCPUStateMSec = rhea::getTimeNowMSec() + 10000;
        glob->bSyncWithCPUResult = false;
        ui->webView->setVisible(false);
        ui->labInfo->setVisible(true);
        this->ui->labInfo->setText("");
        cpubridge::ask_CPU_QUERY_STATE(glob->cpuSubscriber, 0);
        this->show();
        utils::hideMouse();
        break;

    case eForm_boot:
        frmBoot->showMe();
        utils::hideMouse();
        break;

    case eForm_specialActionBeforeGUI:
		priv_scheduleFormChange(eForm_main_showBrowser);
		priv_showForm(eForm_main_showBrowser);
        break;

    case eForm_main_showBrowser:
        {
            char s[1024];
            sprintf_s (s, sizeof(s), "file://%s/current/GUIOpticalBonding/web/startup.html", rhea::getPhysicalPathToAppFolder());
            priv_loadURL(s);
        }
        break;

    case eForm_oldprog_legacy:
        frmProg = new FormProg(this, glob);
        frmProg->showMe();
        utils::hideMouse();
        break;

    case eForm_newprog:
        priv_loadURLMenuProg(NULL);
        break;

    case eForm_newprog_lavaggioSanitario:
        priv_loadURLMenuProg("page=pageCleaningSanitario");
        break;

    case eForm_newprog_lavaggioMilker:
        priv_loadURLMenuProg("page=pageCleaningMilker");
        break;

    case eForm_newprog_descaling:
        priv_loadURLMenuProg("page=pageDescaling");
        break;
		
	case eForm_newprog_dataAudit:
        priv_loadURLMenuProg("page=pageDataAudit");
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
            case eRetCode_gotoNewMenuProg_descaling: priv_scheduleFormChange(eForm_newprog_descaling); break;
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
            case eRetCode_gotoNewMenuProg_descaling: priv_scheduleFormChange(eForm_newprog_descaling); break;
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
            case eRetCode_gotoNewMenuProg_descaling: priv_scheduleFormChange(eForm_newprog_descaling); break;
			case eRetCode_gotoNewMenuProg_partialDataAudit: priv_scheduleFormChange(eForm_newprog_dataAudit); break;
        }
        break;

    case eForm_oldprog_legacy:
        if (frmProg->onTick() != eRetCode_none)
            priv_scheduleFormChange(eForm_main_syncWithCPU);
        break;

    case eForm_newprog:
    case eForm_newprog_lavaggioMilker:
    case eForm_newprog_lavaggioSanitario:
    case eForm_newprog_descaling:
	case eForm_newprog_dataAudit:
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
    while (rhea::thread::popMsg(glob->cpuSubscriber.hFromMeToSubscriberR, &msg))
    {
        priv_syncWithCPU_onCPUBridgeNotification(msg);
        rhea::thread::deleteMsg(msg);
    }

    //se è un po' che non ricevo lo stato della CPU, lo richiedo
    const u64 timeNowMSec = rhea::getTimeNowMSec();
    if (timeNowMSec > syncWithCPU.nextTimeoutAskCPUStateMSec)
    {
        syncWithCPU.nextTimeoutAskCPUStateMSec = timeNowMSec + 10000;
        cpubridge::ask_CPU_QUERY_STATE(glob->cpuSubscriber, 0);
    }


    if (syncWithCPU.stato == 0)
    {
        this->glob->bSyncWithCPUResult = true;
        syncWithCPU.stato = 99;

        if (this->glob->bSyncWithCPUResult == false || rhea::fs::folderExists(glob->usbFolder) || priv_autoupdate_exists())
            priv_scheduleFormChange (eForm_boot);
        else
            priv_scheduleFormChange (eForm_specialActionBeforeGUI);
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
                cpubridge::ask_CPU_GET_EXTENDED_CONFIG_INFO(glob->cpuSubscriber, 0);
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
            if (0 != (flag1 & cpubridge::sCPUStatus::FLAG1_READY_TO_DELIVER_DATA_AUDIT))
                glob->bCPUEnteredInMainLoop=1;
            if (0 != (flag1 & cpubridge::sCPUStatus::FLAG1_IS_MILKER_ALIVE))
                glob->bIsMilkerAlive=1;
            else
                glob->bIsMilkerAlive=0;
        }
        break;

    case CPUBRIDGE_NOTIFY_READ_VMCDATAFILE_PROGRESS:
        {
            cpubridge::eReadDataFileStatus status;
            u16 totKbSoFar = 0;
            u16 fileID = 0;
            cpubridge::translateNotify_READ_VMCDATAFILE_PROGRESS (msg, &status, &totKbSoFar, &fileID);

            char s[512];
            if (status == cpubridge::eReadDataFileStatus::inProgress)
            {
                sprintf_s (s, sizeof(s), "Downloading VMC Settings... %d Kb", totKbSoFar);
                priv_addText (s);
            }
            else if (status == cpubridge::eReadDataFileStatus::finishedOK)
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

    //ogni tot, chiedo a CPUBridge il suo stato di lock per eventualmente sovraimporre
    //la schermata di "CAUTION! machine locked"
    const u64 timeNowMSec = rhea::getTimeNowMSec();
    if (timeNowMSec >= nextTimeAskForCPULockStatus_msec)
    {
        cpubridge::ask_GET_MACHINE_LOCK_STATUS(glob->cpuSubscriber, 0);
        nextTimeAskForCPULockStatus_msec = timeNowMSec +10000;
    }


    //vediamo se CPUBridge ha qualcosa da dirmi
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg(glob->cpuSubscriber.hFromMeToSubscriberR, &msg))
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
    case CPUBRIDGE_NOTIFY_LOCK_STATUS:
        {
            cpubridge::eLockStatus lockStatus;
            cpubridge::translateNotify_MACHINE_LOCK (msg, &lockStatus);
            if (cpubridge::eLockStatus::unlocked == lockStatus)
                priv_showLockedPanel(false);
            else
                priv_showLockedPanel(true);
        }
        break;

    case CPUBRIDGE_NOTIFY_CPU_INI_PARAM:
        {
            cpubridge::sCPUParamIniziali iniParam;
            cpubridge::translateNotify_CPU_INI_PARAM (msg, &iniParam);
            strcpy (glob->cpuVersion, iniParam.CPU_version);
        }
        break;

    case CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS:
        {
            cpubridge::eRunningSelStatus s = cpubridge::eRunningSelStatus::finished_KO;
            cpubridge::translateNotify_CPU_RUNNING_SEL_STATUS (msg, &s);
            if (s == cpubridge::eRunningSelStatus::finished_OK)
                History::incCounterSelezioni();
        }
        break;

    case CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED:
        {
            cpubridge::eVMCState vmcState;
            u8 vmcErrorCode=0, vmcErrorType=0;
            u16 flag1=0;
            cpubridge::translateNotify_CPU_STATE_CHANGED (msg, &vmcState, &vmcErrorCode, &vmcErrorType, &flag1);

            if (0 != (flag1 & cpubridge::sCPUStatus::FLAG1_READY_TO_DELIVER_DATA_AUDIT))
                glob->bCPUEnteredInMainLoop=1;
            if (0 != (flag1 & cpubridge::sCPUStatus::FLAG1_IS_MILKER_ALIVE))
                glob->bIsMilkerAlive=1;
            else
                glob->bIsMilkerAlive=0;

            //non dovrebbe mai succede che la CPU vada da sola in PROG, ma se succede io faccio apparire il vecchio menu PROG
            if (vmcState == cpubridge::eVMCState::PROGRAMMAZIONE)
                retCode = eRetCode_gotoFormOldMenuProg;
            //questo è il caso in cui la CPU non ha portato a termine un LAV SANITARIO. Spegnendo e riaccendendo la macchina, la
            //CPU va da sola in LAV_SANITARIO e io di conseguenza devo andare nel nuovo menu prog alla pagina corretta
            else if (vmcState == cpubridge::eVMCState::LAVAGGIO_SANITARIO)
                retCode = eRetCode_gotoNewMenuProg_LavaggioSanitario;
            //questo è il caso in cui la CPU non ha portato a termine un LAV SANITARIO del cappucinatore. Funziona come sopra
            else if (vmcState == cpubridge::eVMCState::LAVAGGIO_MILKER_VENTURI || vmcState == cpubridge::eVMCState::LAVAGGIO_MILKER_INDUX)
                retCode = eRetCode_gotoNewMenuProg_lavaggioMilker;
            //come sopra, ma per il descaling
            else if (vmcState == cpubridge::eVMCState::DESCALING)
                retCode = eRetCode_gotoNewMenuProg_descaling;
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
    while (rhea::thread::popMsg(glob->cpuSubscriber.hFromMeToSubscriberR, &msg))
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
    if (currentForm >= eForm_newprog || currentForm == eForm_main_showBrowser)
    {
        QString url = arg1.toString();

        if (currentForm >= eForm_newprog)
        {
            if (url.indexOf("gotoLegacyMenu.html") > 0)
            {
                //dal nuovo menu di programmazione, vogliamo andare in quello vecchio!
                retCode = eRetCode_gotoFormOldMenuProg;
            }
            else if (url.indexOf("gotoHMI.html") > 0)
            {
                //dal nuovo menu di programmazione, vogliamo tornare alla GUI utente
                retCode = eRetCode_gotoFormBrowser;
            }
        }
        else if (currentForm == eForm_main_showBrowser)
        {
            if (url.indexOf("gotoMilkerCleaning.html") > 0)
            {
                //dalla GUI utente al nuovo menu prog > lavaggio milker
                retCode = eRetCode_gotoNewMenuProg_lavaggioMilker;
            }
            else if (url.indexOf("gotoPartialDataAudit.html") > 0)
            {
                //dalla GUI utente al nuovo menu prog > data audit
                retCode = eRetCode_gotoNewMenuProg_partialDataAudit;
            }
        }		
    }
}
