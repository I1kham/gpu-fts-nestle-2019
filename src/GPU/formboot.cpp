#include "header.h"
#include "formboot.h"
#include "ui_formboot.h"
#include "history.h"
#include "../rheaAppLib/rheaAppUtils.h"
#include "../rheaExternalSerialAPI/ESAPI.h"
#include <QWidget>
#include <QDir>
#include <QFile>
#include <QTime>
#include <QTimer>
#include <QMessageBox>
#include <QThread>
#include <QProcess>
#include <unistd.h>
#include "Utils.h"


//********************************************************
FormBoot::FormBoot(QWidget *parent, sGlobal *glob) :
    QDialog(parent), ui(new Ui::FormBoot)
{
    this->glob = glob;
    retCode = eRetCode_none;
    dwnloadDataAuditCallBack = eDwnloadDataAuditCallBack_none;
    upldDA3CallBack = eUploadDA3CallBack_none;
    upldCPUFWCallBack = eUploadCPUFWCallBack_none;
    bBtnStartVMCEnabled = true;
    sizeInBytesOfCurrentFileUnpload = 0;

    autoupdate.isRunning = false;
    memset (autoupdate.cpuFileName, 0, sizeof(autoupdate.cpuFileName));
    memset (autoupdate.da3FileName, 0, sizeof(autoupdate.da3FileName));
    memset (autoupdate.guiFolderName, 0, sizeof(autoupdate.guiFolderName));

    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);


    ui->frameFileList->setVisible(false);

    ui->labWait->setVisible(false);

    //CPU message/status
    ui->labCPUMessage->setText("");
    ui->labCPUStatus->setText ("");

    //Software version
    ui->labVersion_CPU->setText("");
    ui->labVersion_CPUMasterName->setText("");
    ui->labVersion_GPU->setText("");
    ui->labVersion_protocol->setText("");
    ui->labGPU_buildDate->setText ("Build date: " __DATE__ " " __TIME__);

    //modulo ESAPI
    ui->labESAPI->setText ("ESAPI module: <span style='color:#fff'>none</span>");

    //Bottoni
    ui->btnInstall_languages->setVisible(false);

    ui->framePleaseWait->setVisible(false);
    ui->framePleaseWait->move (0, 250);


    ui->frameAutoUpdate->setVisible(false);


    priv_updateLabelInfo();
}

//***********************************************************
FormBoot::~FormBoot()
{
    delete ui;
}

//************************************************************
void FormBoot::priv_enableButton (QPushButton *btn, bool bEnabled)
{
    btn->setEnabled(bEnabled);
    if (bEnabled)
        btn->setStyleSheet ("background-color:#656565; color:#000; border-radius:10px;");
    else
        btn->setStyleSheet ("background-color:#151515; color:#333; border-radius:10px;");
}

//*******************************************
void FormBoot::showMe()
{
    retCode = eRetCode_none;
    priv_pleaseWaitSetText("");
    priv_pleaseWaitHide();

    //Download di data-audit e dello zip diagnostico è possibile solo quando la CPU è pronta.
    //E' la CPU che ad un certo punto comunica che è pronta per eventuali download del data-audit. Fino ad allora, disabilito
    //la possibilità di richiederlo (e dato che lo zippone deve include il data-audit, disabilito anche lui)
    priv_enableButton(ui->btnDownload_audit, false);
    priv_enableButton(ui->btnDownload_diagnostic, false);

    if (glob->bSyncWithCPUResult)
    {
        //se entriamo qui. vuol dire che la CPU attualmente installata è "buona", ovvero è una CPU fusion 2, quindi possiamo procedere
        //normalmente
        cpubridge::ask_CPU_QUERY_INI_PARAM(glob->cpuSubscriber, 0);
        cpubridge::ask_CPU_QUERY_STATE(glob->cpuSubscriber, 0);
        cpubridge::ask_CPU_SHOW_STRING_VERSION_AND_MODEL(glob->cpuSubscriber, 0);
        cpubridge::ask_CPU_STRING_VERSION_AND_MODEL(glob->cpuSubscriber, 0);

        //se esite la cartella AUTOF2, parto con l'autoupdate
        if (priv_autoupdate_exists())
            priv_autoupdate_showForm();
    }
    else
    {
        //se arriviamo qui è perchè ci sono stati dei problemi con la sincronizzazione con la CPU.
        //In generale questo vuol dire che la CPU non è installata oppure è una versione non compatibile.
        //Disbailito il pulsante di START VMC e chiedo di aggiornare il FW di CPU perchè non è possibile procedere oltre senza una
        //CPU adeguata
        priv_foreverDisableBtnStartVMC();
        priv_pleaseWaitSetError("WARNING: There was an error during synchronization with the CPU.<br>Please upgrade the CPU FW to a compatible version.");

        //in ogni caso, se esite la cartella AUTOF2, parto con l'autoupdate perchè probabilmente nella cartella AUTOF2 c'è una CPU buona da caricare
        if (priv_autoupdate_exists())
            priv_autoupdate_showForm();

    }
    priv_updateLabelInfo();
    this->show();
}

//*******************************************
void FormBoot::priv_updateLabelInfo()
{
    char s[512];
    OSFileFind ff;

    //GPU Version
    //sprintf_s (s, sizeof(s), );
    ui->labVersion_GPU->setText("<b>GPU</b>: <span style='color:#fff'>" GPU_VERSION "</span>");

    //CPU version + protocol version sono aggiornate on the fly mano mano che si ricevono i messaggi da CPU

    //Installed files: CPU
    ui->labInstalled_CPU->setText("CPU:");
    if (rhea::fs::findFirst (&ff, glob->last_installed_cpu, (const u8*)"*.mhx"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
            {
                sprintf_s (s, sizeof(s), "<b>CPU</b>: <span style='color:#fff'>%s</span>", rhea::fs::findGetFileName(ff));
                ui->labInstalled_CPU->setText(s);
                break;
            }
        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);
    }

    //Installed files: DA3
    ui->labInstalled_DA3->setText("VMC SETTINGS:");
    ui->labInstalled_DA3_DataUM->setText("");
    if (rhea::fs::findFirst (&ff, glob->last_installed_da3, (const u8*)"*.da3"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
            {
                sprintf_s (s, sizeof(s), "<b>VMC SETTINGS</b>: <span style='color:#fff'>%s</span>", rhea::fs::findGetFileName(ff));
                ui->labInstalled_DA3->setText(s);

                sprintf_s(s, sizeof(s), "%s/last_installed/da3/dateUM.bin", rhea::getPhysicalPathToAppFolder());
                FILE *f = fopen(s,"rb");
                if (NULL != f)
                {
                    u64 u;
                    fread(&u,sizeof(u64),1,f);
                    fclose(f);

                    rhea::DateTime dt;
                    dt.setFromInternalRappresentation(u);

                    dt.formatAs_YYYYMMDDHHMMSS(s, sizeof(s), ' ', '/', ':');
                    ui->labInstalled_DA3_DataUM->setText (QString("last updated ") +s);
                }
                break;
            }
        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);
    }

    //Installed files: GUI
    ui->labInstalled_GUI->setText("GUI:");
    if (rhea::fs::findFirst (&ff, glob->last_installed_gui, (const u8*)"*.rheagui"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
            {
                u8 onlyFileName[256];
                rhea::fs::extractFileNameWithoutExt (rhea::fs::findGetFileName(ff), onlyFileName, sizeof(onlyFileName));
                sprintf_s (s, sizeof(s), "<b>GUI</b>: <span style='color:#fff'>%s</span>", onlyFileName);
                ui->labInstalled_GUI->setText(s);
                break;
            }
        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);
    }


    //Installed files: Manual (mi interessa solo il nome del primo folder valido)
    ui->labInstalled_Manual->setText("MANUAL:");
    if (rhea::fs::findFirst (&ff, glob->last_installed_manual, (const u8*)"*.*"))
    {
        do
        {
            if (rhea::fs::findIsDirectory(ff))
            {
                const u8 *folderName = rhea::fs::findGetFileName(ff);
                if (folderName[0] != '.')
                {
                    sprintf_s (s, sizeof(s), "<b>MANUAL</b>: <span style='color:#fff'>%s</span>", folderName);
                    ui->labInstalled_Manual->setText(s);
                    break;
                }
            }
        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);
    }

    //modulo rasPI
    if (glob->esapiModule.verMajor != 0x00)
    {
        if (esapi::eExternalModuleType_none == glob->esapiModule.moduleType)
        {
            sprintf_s (s, sizeof(s), "ESAPI: <span style='color:#fff'>API v%d.%d</span>", glob->esapiModule.verMajor, glob->esapiModule.verMinor);
            ui->labESAPI->setText (s);
        }
        else
        {
            char moduleName[32];
            switch (glob->esapiModule.moduleType)
            {
            default:
                sprintf_s (moduleName, sizeof(moduleName), "%d", glob->esapiModule.moduleType);
                break;

            case esapi::eExternalModuleType_rasPI_wifi_REST:
                sprintf_s (moduleName, sizeof(moduleName), "rasPI");
                break;
            }
            sprintf_s (s, sizeof(s), "ESAPI module: <span style='color:#fff'>%s, v%d.%d</span>", moduleName, glob->esapiModule.verMajor, glob->esapiModule.verMinor);
            ui->labESAPI->setText (s);
        }
    }
}

//*******************************************
void FormBoot::priv_syncUSBFileSystem (u64 minTimeMSecToWaitMSec)
{
    sync();

    char s[512];
    sprintf_s (s, sizeof(s), "sync -d %s", glob->usbFolder);
    system(s);

    sprintf_s (s, sizeof(s), "sync -f %s", glob->usbFolder);
    system(s);

    sprintf_s (s, sizeof(s), "%s/waitUSBSync.dat", glob->usbFolder);
    rhea::fs::fileDelete((const u8*)s);

    FILE *f = fopen (s, "wb");
    fwrite (s, 1, sizeof(s), f);
    fwrite (s, 1, sizeof(s), f);
    fwrite (s, 1, sizeof(s), f);
    fflush(f);
    fclose (f);

    utils::waitAndProcessEvent(minTimeMSecToWaitMSec);

    sync();

    sprintf_s (s, sizeof(s), "sync -d %s", glob->usbFolder);
    system(s);

    sprintf_s (s, sizeof(s), "sync -f %s", glob->usbFolder);
    system(s);

    sprintf_s (s, sizeof(s), "%s/waitUSBSync.dat", glob->usbFolder);
    for (u8 i=0; i<10; i++)
    {
        FILE *f = fopen (s, "rb");
        const u64 fsize = rhea::fs::filesize(f);
        fclose(f);
        if (fsize < 1500)
        {
            sync();
            utils::waitAndProcessEvent(500);
        }
        else
            break;
    }

    rhea::fs::fileDelete((const u8*)s);
}

//*******************************************
eRetCode FormBoot::onTick()
{
    if (retCode != eRetCode_none)
        return retCode;

    //vediamo se CPUBridge ha qualcosa da dirmi
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg(glob->cpuSubscriber.hFromMeToSubscriberR, &msg))
    {
        priv_onCPUBridgeNotification(msg);
        rhea::thread::deleteMsg(msg);
    }

    if (autoupdate.isRunning)
        priv_autoupdate_onTick();

    return eRetCode_none;
}

/**************************************************************************
 * priv_onCPUBridgeNotification
 *
 * E' arrivato un messaggio da parte di CPUBrdige sulla msgQ dedicata (ottenuta durante la subscribe di this a CPUBridge).
 */
void FormBoot::priv_onCPUBridgeNotification (rhea::thread::sMsg &msg)
{
    const u16 handlerID = (msg.paramU32 & 0x0000FFFF);
    assert (handlerID == 0);

    const u16 notifyID = (u16)msg.what;
    switch (notifyID)
    {
    case CPUBRIDGE_NOTITFY_GET_CPU_STRING_MODEL_AND_VER:
        {
            u16  s[64];
            cpubridge::translateNotify_CPU_STRING_VERSION_AND_MODEL(msg, s, sizeof(s));

            QString sss ="";
            u32 i=0;
            while (s[i] != 0x00)
            {
                sss += QChar(s[i]);
                i++;
            }
            sss.replace("  ", " "); sss.replace("  ", " "); sss.replace("  ", " "); sss.replace("  ", " ");
            sss.replace("  ", " "); sss.replace("  ", " "); sss.replace("  ", " "); sss.replace("  ", " ");
            ui->labVersion_CPUMasterName->setText(sss);
        }
        break;

    case CPUBRIDGE_NOTIFY_CPU_INI_PARAM:
        {
            char s[256];
            cpubridge::sCPUParamIniziali iniParam;
            cpubridge::translateNotify_CPU_INI_PARAM (msg, &iniParam);

            sprintf_s (s, sizeof(s), "<b>CPU</b>: <span style='color:#fff'>%s</span>", iniParam.CPU_version);
            ui->labVersion_CPU->setText (s);

            sprintf_s (s, sizeof(s), "<b>Protocol ver</b>: <span style='color:#fff'>%d</span>", iniParam.protocol_version);
            ui->labVersion_protocol->setText(s);

            strcpy (glob->cpuVersion, iniParam.CPU_version);
        }
        break;

    case CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED:
        {
            cpubridge::eVMCState vmcState;
            u8 vmcErrorCode = 0, vmcErrorType = 0;
            u16 flag1 = 0;
            cpubridge::translateNotify_CPU_STATE_CHANGED (msg, &vmcState, &vmcErrorCode, &vmcErrorType, &flag1);
            ui->labCPUStatus->setText (rhea::app::utils::verbose_eVMCState (vmcState));

            if (0 == (flag1 & cpubridge::sCPUStatus::FLAG1_READY_TO_DELIVER_DATA_AUDIT))
            {
                priv_enableButton (ui->btnDownload_audit, false);
                priv_enableButton(ui->btnDownload_diagnostic, false);
            }
            else
            {
                glob->bCPUEnteredInMainLoop=1;
                priv_enableButton (ui->btnDownload_audit, true);
                priv_enableButton(ui->btnDownload_diagnostic, true);
            }
            if (0 != (flag1 & cpubridge::sCPUStatus::FLAG1_IS_MILKER_ALIVE))
                glob->bIsMilkerAlive=1;
            else
                glob->bIsMilkerAlive=0;

            //non dovrebbe mai succede che la CPU vada da sola in PROG, ma se succede io faccio apparire il vecchio menu PROG
            if (vmcState == cpubridge::eVMCState_PROGRAMMAZIONE)
                retCode = eRetCode_gotoFormOldMenuProg;

            /* GIX: voglio che il form boot rimanga nel form boot anche in caso di lavaggi sanitari pendenti
             *
             *
            //questo è il caso in cui la CPU non ha portato a termine un LAV SANITARIO. Spegnendo e riaccendendo la macchina, la
            //CPU va da sola in LAV_SANITARIO e io di conseguenza devo andare nel nuovo menu prog alla pagina corretta
            else if (vmcState == cpubridge::eVMCState_LAVAGGIO_SANITARIO)
                retCode = eRetCode_gotoNewMenuProg_LavaggioSanitario;
            //come sopra ma per il cappucinatore
            else if (vmcState == cpubridge::eVMCState_LAVAGGIO_MILKER)
                retCode = eRetCode_gotoNewMenuProg_lavaggioMilker;
            */
        }
        break;

    case CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE:
        {
            cpubridge::sCPULCDMessage cpuMsg;
            cpubridge::translateNotify_CPU_NEW_LCD_MESSAGE(msg, &cpuMsg);

            u32 i=0;
            while (cpuMsg.utf16LCDString[i] != 0x00)
            {
                msgCPU[i] = cpuMsg.utf16LCDString[i];
                i++;
            }
            msgCPU[i] = 0;
            ui->labCPUMessage->setText(QString(msgCPU, -1));
        }
        break;

    case CPUBRIDGE_NOTIFY_BTN_PROG_PRESSED:
        //l'utente ha premuto il btn PROG, devo andare in programmazione
        break;

    case CPUBRIDGE_NOTIFY_READ_DATA_AUDIT_PROGRESS:
        switch (dwnloadDataAuditCallBack)
        {
        default: break;
        case eDwnloadDataAuditCallBack_btn:         priv_on_btnDownload_audit_download(msg); break;
        case eDwnloadDataAuditCallBack_service:     priv_on_btnDownload_diagnostic_downloadDataAudit(msg); break;
        }
        break;

    case CPUBRIDGE_NOTIFY_WRITE_VMCDATAFILE_PROGRESS:
        switch (upldDA3CallBack)
        {
        default: break;
        case eUploadDA3CallBack_btn: priv_on_btnInstall_DA3_upload(msg); break;
        case eUploadDA3CallBack_auto: priv_autoupdate_onDA3_upload(msg); break;
        }
        break;

    case CPUBRIDGE_NOTIFY_WRITE_CPUFW_PROGRESS:
        switch (upldCPUFWCallBack)
        {
        default: break;
        case eUploadCPUFWCallBack_btn: priv_on_btnInstall_CPU_upload(msg); break;
        case eUploadCPUFWCallBack_auto: priv_autoupdate_onCPU_upload(msg); break;
        case eUploadCPUFWCallBack_ESAPIGUI: priv_uploadESAPI_GUI(msg); break;
        }
        break;
    }
}



//***********************************************************
void FormBoot::priv_foreverDisableBtnStartVMC()
{
    if (!bBtnStartVMCEnabled)
        return;
    bBtnStartVMCEnabled = false;
    ui->buttonStart->setStyleSheet("QPushButton {color: #FF0000; border:  none}");
    ui->buttonStart->setText ("Please restart\nVMC");
}

//**********************************************************************
void FormBoot::priv_startDownloadDataAudit (eDwnloadDataAuditCallBack mode)
{
    dwnloadDataAuditCallBack = mode;
    cpubridge::ask_READ_DATA_AUDIT (glob->cpuSubscriber, 0);
}

void FormBoot::priv_startUploadDA3 (eUploadDA3CallBack mode, const u8 *fullFilePathAndName)
{
    upldDA3CallBack = mode;
    cpubridge::ask_WRITE_VMCDATAFILE (glob->cpuSubscriber, 0, fullFilePathAndName);
}

void FormBoot::priv_startUploadCPUFW (eUploadCPUFWCallBack mode, const u8 *fullFilePathAndName)
{
    upldCPUFWCallBack = mode;
    priv_foreverDisableBtnStartVMC();
    sizeInBytesOfCurrentFileUnpload = rhea::fs::filesize(fullFilePathAndName);
    cpubridge::ask_WRITE_CPUFW (glob->cpuSubscriber, 0, fullFilePathAndName);
}

//*******************************************
void FormBoot::on_buttonStart_clicked()
{
    if (bBtnStartVMCEnabled == false)
        return;

    //verifico che ci sia una GUI installata
    u8 s[256];
    sprintf_s ((char*)s, sizeof(s), "%s/current/gui/template.rheagui", rhea::getPhysicalPathToAppFolder());
    if (!rhea::fs::fileExists(s))
    {
        priv_pleaseWaitShow("");
        priv_pleaseWaitSetError("ERROR: GUI is missing. Please load a new GUI");
        priv_pleaseWaitHide();
        return;
    }

    priv_pleaseWaitShow("Starting VMC...");

#ifdef PLATFORM_YOCTO_EMBEDDED
    sprintf_s ((char*)s, sizeof(s), "umount -f %s", USB_MOUNTPOINT);
    system((const char*)s);
#endif

    //se ESAPI::rasPI esiste, attivo la sua interfaccia web
    if (glob->esapiModule.moduleType == esapi::eExternalModuleType_rasPI_wifi_REST)
        esapi::ask_RASPI_START (glob->esapiSubscriber, (u32)0);

    retCode = eRetCode_gotoFormBrowser;
}


//**********************************************************************
void FormBoot::priv_pleaseWaitShow(const char *message)
{
    ui->line_3->setVisible(false);
    ui->line_4->setVisible(false);
    ui->frameInstallBtn->setVisible(false);
    ui->frameDownloadBtn->setVisible(false);
    ui->buttonStart->setVisible(false);
    ui->framePleaseWait->setVisible(true);

    priv_pleaseWaitSetText(message);
}

//**********************************************************************
void FormBoot::priv_pleaseWaitHide ()
{
    ui->line_3->setVisible(true);
    ui->line_4->setVisible(true);
    ui->framePleaseWait->setVisible(false);
    ui->frameInstallBtn->setVisible(true);
    ui->frameDownloadBtn->setVisible(true);
    ui->buttonStart->setVisible(true);
}

//**********************************************************************
void FormBoot::priv_pleaseSetTextWithColor (const char *message, const char *bgColor, const char *textColor)
{
    if (NULL == message)
        ui->labWait->setVisible(false);
    else if (message[0] == 0x00)
        ui->labWait->setVisible(false);
    else
    {
        char s[96];
        sprintf_s (s, sizeof(s), "QLabel { background-color:%s; color:%s; }", bgColor, textColor);
        ui->labWait->setStyleSheet(s);
        ui->labWait->setVisible(true);
        ui->labWait->setText(message);
    }
    QApplication::processEvents();
}

void FormBoot::priv_pleaseWaitSetText (const char *message)             { priv_pleaseSetTextWithColor (message, "#9b9", "#fff"); }
void FormBoot::priv_pleaseWaitSetOK (const char *message)               { priv_pleaseSetTextWithColor (message, "#43b441", "#fff"); }
void FormBoot::priv_pleaseWaitSetError (const char *message)            { priv_pleaseSetTextWithColor (message, "#f00", "#fff"); }





//**********************************************************************
void FormBoot::priv_fileListShow (eFileListMode mode)
{
    fileListShowMode = mode;
    ui->buttonStart->setVisible(false);
    ui->frameFileList->move(10, 35);
    ui->frameFileList->setVisible(true);
    ui->frameFileList->raise();
    ui->lbFileList->clear();
}

//**********************************************************************
void FormBoot::priv_fileListHide()
{
    ui->frameFileList->setVisible(false);
    ui->buttonStart->setVisible(true);
}

//**********************************************************************
void FormBoot::priv_fileListPopulate(const u8 *pathNoSlash, const u8 *jolly, bool bClearList)
{
    if (bClearList)
        ui->lbFileList->clear();

    OSFileFind ff;

    if (rhea::fs::findFirst(&ff, pathNoSlash, jolly))
    {
        do
        {
            if (rhea::fs::findIsDirectory(ff))
                continue;
            const char *fname = (const char*)rhea::fs::findGetFileName(ff);
            ui->lbFileList->addItem(fname);
        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);

        if (ui->lbFileList->count())
            ui->lbFileList->item(0)->setSelected(true);
    }
}

void FormBoot::on_lbFileList_doubleClicked(const QModelIndex &index UNUSED_PARAM)                { on_btnOK_clicked(); }
void FormBoot::on_btnCancel_clicked()                                               { priv_fileListHide(); }

//**********************************************************************
void FormBoot::on_btnOK_clicked()
{
    if (ui->lbFileList->selectedItems().count() == 0)
        return;

    QListWidgetItem *item = ui->lbFileList->selectedItems().at(0);
    QString srcFilename = item->text();
    char src[512];

    switch (fileListShowMode)
    {
    default:
        priv_fileListHide();
        break;

    case eFileListMode_DA3:
        sprintf_s (src, sizeof(src), "%s/%s", glob->usbFolder_VMCSettings, srcFilename.toStdString().c_str());
        priv_fileListHide();
        priv_on_btnInstall_DA3_onFileSelected((const u8*)src);
        break;

    case eFileListMode_Manual:
        sprintf_s (src, sizeof(src), "%s/%s", glob->usbFolder_Manual, srcFilename.toStdString().c_str());
        priv_fileListHide();
        priv_uploadManual((const u8*)src);
        break;

    case eFileListMode_GUI:
        sprintf_s (src, sizeof(src), "%s/%s", glob->usbFolder_GUI, srcFilename.toStdString().c_str());
        priv_fileListHide();
        priv_uploadGUI((const u8*)src);
        break;

    case eFileListMode_CPU:
        sprintf_s (src, sizeof(src), "%s/%s", glob->usbFolder_CPUFW, srcFilename.toStdString().c_str());
        priv_fileListHide();
        priv_on_btnInstall_CPU_onFileSelected((const u8*)src);
        break;
    }

}



/**********************************************************************
 *
 * copia tutti i file *.lng dalla USB all'HD locale
 *
 */
void FormBoot::on_btnInstall_languages_clicked()
{
    priv_langCopy (glob->usbFolder_Lang, glob->current_lang, 10000);
}

//**********************************************************************
bool FormBoot::priv_langCopy (const u8 *srcFolder, const u8 *dstFolder, u32 timeToWaitDuringCopyFinalizingMSec)
{
    priv_pleaseWaitShow("copying files...");

    QApplication::processEvents();

    //se la directory dst non esiste, la creo, se esiste, la svuoto
    {
        QDir root_dir((const char*)dstFolder);
        if (root_dir.exists())
            root_dir.removeRecursively();
        root_dir.mkpath(".");
    }

    //copia
    bool ret = rhea::fs::folderCopy (srcFolder, dstFolder);

    if (ret)
    {
        priv_pleaseWaitSetText("Finalizing copy...");
        utils::waitAndProcessEvent (timeToWaitDuringCopyFinalizingMSec);
        priv_pleaseWaitSetOK("SUCCESS");
    }
    else
        priv_pleaseWaitSetError("Error during file copy!");

    priv_pleaseWaitHide();
    return (ret);
}



/**********************************************************************
 * Install manual
 */
void FormBoot::on_btnInstall_manual_clicked()
{
    priv_fileListShow(eFileListMode_Manual);
    //priv_fileListPopulate(glob->usbFolder_Manual, "*.pdf", true);

    //popola la lista
    ui->lbFileList->clear();
    OSFileFind ff;
    char s[512];
    if (rhea::fs::findFirst(&ff, glob->usbFolder_Manual, (const u8*)"*.*"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
                continue;
            const char *dirName = (const char*)rhea::fs::findGetFileName(ff);
            if (dirName[0] == '.')
                continue;

            sprintf_s (s, sizeof(s), "%s/%s/index.html", glob->usbFolder_Manual, dirName);
            if (rhea::fs::fileExists((const u8*)s))
            {
                ui->lbFileList->addItem(dirName);
            }


        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);

        if (ui->lbFileList->count())
            ui->lbFileList->item(0)->setSelected(true);
    }
}

void FormBoot::priv_uploadManual (const u8 *srcFullFolderPath)
{
    u8 srcOnlyFolderName[256];
    u8 s[256];
    rhea::fs::extractFileNameWithoutExt (srcFullFolderPath, srcOnlyFolderName, sizeof(srcOnlyFolderName));
    if (strcmp((const char*)srcOnlyFolderName,"REMOVE_MANUAL") == 0)
    {
        priv_pleaseWaitShow("Removing manual...");
        //elimino la roba attualmente installata
        rhea::fs::deleteAllFileInFolderRecursively (glob->last_installed_manual, false);
        priv_pleaseWaitSetOK("SUCCESS.<br>Manual removed");
    }
    else
    {
        priv_pleaseWaitShow("Installing manual...");

        //elimino la roba attualmente installata
        rhea::fs::deleteAllFileInFolderRecursively (glob->last_installed_manual, false);


        sprintf_s ((char*)s, sizeof(s), "%s/%s", glob->last_installed_manual, srcOnlyFolderName);
        rhea::fs::folderCreate (s);

        //copio tutto il folder src nel folder in macchina
        if (!rhea::fs::folderCopy(srcFullFolderPath, s))
            priv_pleaseWaitSetError("ERROR copying files");
        else
            priv_pleaseWaitSetOK("SUCCESS.<br>Manual installed");
    }

    priv_pleaseWaitHide();
    priv_updateLabelInfo();
}



/**********************************************************************
 * Install DA3
 */
void FormBoot::on_btnInstall_DA3_clicked()
{
    //chiedo all'utente quale file vuole uppare...
    priv_fileListShow(eFileListMode_DA3);
    priv_fileListPopulate(glob->usbFolder_VMCSettings, (const u8*)"*.da3", true);
}

void FormBoot::priv_on_btnInstall_DA3_onFileSelected (const u8 *fullFilePathAndName)
{
    priv_pleaseWaitShow("Installing VMC Settings...");
    priv_startUploadDA3 (eUploadDA3CallBack_btn, fullFilePathAndName);
}

void FormBoot::priv_on_btnInstall_DA3_upload (rhea::thread::sMsg &msg)
{
    cpubridge::eWriteDataFileStatus status;
    u16 totKbSoFar = 0;
    cpubridge::translateNotify_WRITE_VMCDATAFILE_PROGRESS (msg, &status, &totKbSoFar);

    char s[512];
    if (status == cpubridge::eWriteDataFileStatus_inProgress)
    {
        sprintf_s (s, sizeof(s), "Installing VMC Settings...... %d KB", totKbSoFar);
        priv_pleaseWaitSetText (s);
    }
    else if (status == cpubridge::eWriteDataFileStatus_finishedOK)
    {
        upldDA3CallBack = eUploadDA3CallBack_none;
        sprintf_s (s, sizeof(s), "Installing VMC Settings...... SUCCESS");
        priv_pleaseWaitSetOK (s);
        priv_pleaseWaitHide();
        priv_updateLabelInfo();
        priv_foreverDisableBtnStartVMC();
    }
    else
    {
        upldDA3CallBack = eUploadDA3CallBack_none;
        sprintf_s (s, sizeof(s), "Installing VMC Settings...... ERROR: %s", rhea::app::utils::verbose_writeDataFileStatus(status));
        priv_pleaseWaitSetError(s);
        priv_pleaseWaitHide();
        priv_updateLabelInfo();
    }

}



/**********************************************************************
 * Install GUI
 */
void FormBoot::on_btnInstall_GUI_clicked()
{
    priv_fileListShow(eFileListMode_GUI);

    //popola la lista
    ui->lbFileList->clear();
    OSFileFind ff;
    char s[512];
    if (rhea::fs::findFirst(&ff, glob->usbFolder_GUI, (const u8*)"*.*"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
                continue;
            const char *dirName = (const char*)rhea::fs::findGetFileName(ff);
            if (dirName[0] == '.')
                continue;

            sprintf_s (s, sizeof(s), "%s/%s/template.rheagui", glob->usbFolder_GUI, dirName);
            if (rhea::fs::fileExists((const u8*)s))
            {
                ui->lbFileList->addItem(dirName);
            }


        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);

        if (ui->lbFileList->count())
            ui->lbFileList->item(0)->setSelected(true);
    }
}

void FormBoot::priv_uploadGUI (const u8 *srcFullFolderPath)
{
    priv_pleaseWaitShow("Installing GUI...");

    if (!priv_doInstallGUI (srcFullFolderPath))
    {
        priv_pleaseWaitSetError("ERROR copying files");
        priv_pleaseWaitHide();
        priv_updateLabelInfo();
        return;
    }

    //se c'è collegato il modulo ESAPI eExternalModuleType_rasPI_wifi_REST, e se la GUI importata ha una versione "mobile", devo uppare anche quella
    u8 fullMobileGUIPathAndName[1024];
    sizeInBytesOfCurrentFileUnpload = 0;
    if (glob->esapiModule.moduleType == esapi::eExternalModuleType_rasPI_wifi_REST)
    {
        sprintf_s ((char*)fullMobileGUIPathAndName, sizeof(fullMobileGUIPathAndName), "%s/web/mobile.rheazip", glob->current_GUI);
        if (rhea::fs::fileExists(fullMobileGUIPathAndName))
            sizeInBytesOfCurrentFileUnpload = rhea::fs::filesize(fullMobileGUIPathAndName);
        else
            fullMobileGUIPathAndName[0] = 0;
    }

    if (0 == sizeInBytesOfCurrentFileUnpload)
    {
        priv_pleaseWaitSetOK("SUCCESS.<br>GUI installed");
        priv_pleaseWaitHide();
        priv_updateLabelInfo();
        return;
    }


    /*priv_pleaseWaitShow("Uploading mobile GUI to ESAPI module...");
    upldCPUFWCallBack = eUploadCPUFWCallBack_ESAPIGUI;
    priv_foreverDisableBtnStartVMC();
    cpubridge::ask_CPU_RASPI_MITM_Upload_GUI_TS (glob->cpuSubscriber, 0, fullMobileGUIPathAndName);
    */
}

bool FormBoot::priv_doInstallGUI (const u8 *srcFullFolderPath) const
{
    //elimino la roba attualmente installata
    rhea::fs::deleteAllFileInFolderRecursively(glob->current_GUI, false);
    rhea::fs::deleteAllFileInFolderRecursively (glob->last_installed_gui, false);

    //copio la GUI nella cartella locale
    u8 srcOnlyFolderName[256];
    rhea::fs::extractFileNameWithoutExt (srcFullFolderPath, srcOnlyFolderName, sizeof(srcOnlyFolderName));
    if (!rhea::fs::folderCopy(srcFullFolderPath, glob->current_GUI))
        return false;

    //creo un file con il nome del folder e lo salvo in last_installed_gui in modo da poter visualizzare il "nome" dell'ultima gui installata
    char s[512];
    sprintf_s (s, sizeof(s), "%s/%s.rheagui", glob->last_installed_gui, srcOnlyFolderName);
    FILE *f = fopen (s, "wt");
    {
        rhea::DateTime dt;
        dt.setNow();
        dt.formatAs_YYYYMMDDHHMMSS(s, sizeof(s), ' ', '/', ':');
        fprintf (f, "%s", s);
    }
    fclose(f);

    return true;
}

void FormBoot::priv_uploadESAPI_GUI (rhea::thread::sMsg &msg)
{
    cpubridge::eWriteCPUFWFileStatus status;
    u16 param = 0;
    cpubridge::translateNotify_WRITE_CPUFW_PROGRESS (msg, &status, &param);

    char s[512];
    if (status == cpubridge::eWriteCPUFWFileStatus_inProgress)
    {
        sprintf_s (s, sizeof(s), "Installing GUI on WIFI module... %d/%d KB", param, (sizeInBytesOfCurrentFileUnpload>>10));
        priv_pleaseWaitSetText (s);
    }
    else if (status == cpubridge::eWriteCPUFWFileStatus_finishedOK)
    {
        upldCPUFWCallBack = eUploadCPUFWCallBack_none;
        sprintf_s (s, sizeof(s), "Installing GUI on WIFI module... file upload SUCCESS, now waiting for unzip");
        priv_pleaseWaitSetText (s);
    }
    else
    {
        upldCPUFWCallBack = eUploadCPUFWCallBack_none;
        sprintf_s (s, sizeof(s), "Installing GUI on WIFI module... ERROR: %s [%d]", rhea::app::utils::verbose_WriteCPUFWFileStatus(status), param);
        priv_pleaseWaitSetError(s);
        priv_pleaseWaitHide();
        priv_updateLabelInfo();
    }
}

void FormBoot::priv_uploadESAPI_GUI_unzipped(rhea::thread::sMsg &msg)
{
    /*bool bSuccess=false;
    cpubridge::translateNotify_CPU_RASPI_MITM_Upload_GUI_TS (msg, &bSuccess);
    if (bSuccess)
        priv_pleaseWaitSetOK ("Installing GUI on WIFI module... SUCCESS");
    else
        priv_pleaseWaitSetError ("Installing GUI on WIFI module... Error while unzipping");
    priv_pleaseWaitHide();
    priv_updateLabelInfo();
    */
}


/************************************************************+
 * Install CPU FW
 */
void FormBoot::on_btnInstall_CPU_clicked()
{
    //chiedo all'utente di scegliere il file
    priv_fileListShow(eFileListMode_CPU);
    priv_fileListPopulate(glob->usbFolder_CPUFW, (const u8*)"*.mhx", true);
}

void FormBoot::priv_on_btnInstall_CPU_onFileSelected (const u8 *fullFilePathAndName)
{
    priv_pleaseWaitShow("Installing CPU FW...");
    priv_startUploadCPUFW (eUploadCPUFWCallBack_btn, fullFilePathAndName);
}

void FormBoot::priv_on_btnInstall_CPU_upload (rhea::thread::sMsg &msg)
{
    cpubridge::eWriteCPUFWFileStatus status;
    u16 param = 0;
    cpubridge::translateNotify_WRITE_CPUFW_PROGRESS (msg, &status, &param);

    char s[512];
    if (status == cpubridge::eWriteCPUFWFileStatus_inProgress_erasingFlash)
    {
        priv_pleaseWaitSetText ("Installing CPU FW...erasing flash");
    }
    else if (status == cpubridge::eWriteCPUFWFileStatus_inProgress)
    {
        sprintf_s (s, sizeof(s), "Installing CPU FW... %d/%d KB", param, (sizeInBytesOfCurrentFileUnpload>>10));
        priv_pleaseWaitSetText (s);
    }
    else if (status == cpubridge::eWriteCPUFWFileStatus_finishedOK)
    {
        upldCPUFWCallBack = eUploadCPUFWCallBack_none;
        sprintf_s (s, sizeof(s), "Installing CPU FW... SUCCESS, please restart the machine");
        priv_pleaseWaitSetOK (s);
        priv_pleaseWaitHide();
        priv_updateLabelInfo();
    }
    else
    {
        upldCPUFWCallBack = eUploadCPUFWCallBack_none;
        sprintf_s (s, sizeof(s), "Installing CPU FW... ERROR: %s [%d]", rhea::app::utils::verbose_WriteCPUFWFileStatus(status), param);
        priv_pleaseWaitSetError(s);
        priv_pleaseWaitHide();
        priv_updateLabelInfo();
    }
}

/**********************************************************************
 * Download DA3
 */
void FormBoot::on_btnDownload_DA3_clicked()
{
    priv_pleaseWaitShow("Downloading VMC Settings...");


    //recupero il nome del file del last_installed da3
    char lastInstalledDa3FileName[256];
    lastInstalledDa3FileName[0] = 0x00;
    OSFileFind ff;
    if (rhea::fs::findFirst (&ff, glob->last_installed_da3, (const u8*)"*.da3"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
            {
                strcpy (lastInstalledDa3FileName, (const char*)rhea::fs::findGetFileName(ff));
                break;
            }
        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);
    }

    if (lastInstalledDa3FileName[0] == 0x00)
        sprintf_s (lastInstalledDa3FileName, sizeof(lastInstalledDa3FileName), "vmcDataFile.da3");

    char dst[512];
    rhea::fs::folderCreate(glob->usbFolder_VMCSettings);

    //se il file dst esiste già , aggiungo data e ora al nome file
    sprintf_s (dst, sizeof(dst), "%s/%s", glob->usbFolder_VMCSettings, lastInstalledDa3FileName);
    if (rhea::fs::fileExists((const u8*)dst))
    {
        rhea::DateTime dt;
        char data[64];
        dt.setNow();
        dt.formatAs_YYYYMMDDHHMMSS (data, sizeof(data), '-', 0x00, 0x00);
        sprintf_s (dst, sizeof(dst), "%s/%s-%s", glob->usbFolder_VMCSettings, data, lastInstalledDa3FileName);
    }


    //il file sorgente è l'attuale da3 effettivamente utilizzato dalla macchina
    char src[512];
    sprintf_s (src, sizeof(src), "%s/vmcDataFile.da3", glob->current_da3);

    //copio
    if (!rhea::fs::fileCopy((const u8*)src, (const u8*)dst))
    {
        priv_pleaseWaitSetError("Error copying file to USB");
    }
    else
    {
        priv_pleaseWaitSetText("Finalizing copy...");
        priv_syncUSBFileSystem(5000);

        rhea::fs::extractFileNameWithExt((const u8*)dst, (u8*)src, sizeof(src));
        sprintf_s (dst, sizeof(dst), "SUCCESS.<br>The file <b>%s</b> has been copied to your USB pendrive in the folder rhea/rheaData", src);
        priv_pleaseWaitSetOK (dst);
    }

    priv_pleaseWaitHide();
}


/************************************************************+
 * Download GUI
 */
void FormBoot::on_btnDownload_GUI_clicked()
{
    priv_pleaseWaitShow("Downloading GUI...");


    //recupero il nome della GUI dal file in last_installed/gui
    char lastInstalledGUIName[256];
    lastInstalledGUIName[0] = 0x00;
    OSFileFind ff;
    if (rhea::fs::findFirst (&ff, glob->last_installed_gui, (const u8*)"*.rheagui"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
            {
                rhea::fs::extractFileNameWithoutExt (rhea::fs::findGetFileName(ff), (u8*)lastInstalledGUIName, sizeof(lastInstalledGUIName));
                break;
            }
        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);
    }

    if (lastInstalledGUIName[0] == 0x00)
    {
        priv_pleaseWaitSetError("ERROR: there's no GUI installed!");
        priv_pleaseWaitHide();
        return;
    }

    char dst[512];
    sprintf_s (dst, sizeof(dst), "%s/%s", glob->usbFolder_GUI, lastInstalledGUIName);
    if (rhea::fs::folderExists((const u8*)dst))
    {
        rhea::DateTime dt;
        char data[64];
        dt.setNow();
        dt.formatAs_YYYYMMDDHHMMSS (data, sizeof(data), '-', 0x00, 0x00);
        sprintf_s (dst, sizeof(dst), "%s/%s-%s", glob->usbFolder_GUI, data, lastInstalledGUIName);
        rhea::fs::folderCreate((const u8*)dst);
    }


    char s[512];
    if (!rhea::fs::folderCopy(glob->current_GUI, (const u8*)dst))
    {
        sprintf_s (s, sizeof(s), "ERROR copying files to [%s]", dst);
        priv_pleaseWaitSetError(s);
    }
    else
    {
        priv_pleaseWaitSetText("Finalizing copy...");
        priv_syncUSBFileSystem(10000);

        rhea::fs::extractFileNameWithoutExt((const u8*)dst, (u8*)s, sizeof(s));
        sprintf_s (dst, sizeof(dst), "SUCCESS.<br>GUI <b>%s</b> have been copied to your USB pendrive in folder rhea/rheaGUI", s);
        priv_pleaseWaitSetOK(dst);
    }


    priv_pleaseWaitHide();
}


/**********************************************************************
 * Download data audit
 */
void FormBoot::on_btnDownload_audit_clicked()
{
    priv_pleaseWaitShow("Downloading data audit...");
    priv_startDownloadDataAudit (eDwnloadDataAuditCallBack_btn);
}

void FormBoot::priv_on_btnDownload_audit_download (rhea::thread::sMsg &msg)
{
    cpubridge::eReadDataFileStatus status;
    u16 totKbSoFar = 0;
    u16 fileID = 0;
    cpubridge::translateNotify_READ_DATA_AUDIT_PROGRESS (msg, &status, &totKbSoFar, &fileID);

    char s[512];
    if (status == cpubridge::eReadDataFileStatus_inProgress)
    {
        sprintf_s (s, sizeof(s), "Downloading data audit... %d KB", totKbSoFar);
        priv_pleaseWaitSetText (s);
    }
    else if (status == cpubridge::eReadDataFileStatus_finishedOK)
    {
        dwnloadDataAuditCallBack = eDwnloadDataAuditCallBack_none;
        sprintf_s (s, sizeof(s), "Downloading data audit finished. Copying to USB folder, please wait...");
        priv_pleaseWaitSetText (s);

        //se non esiste, creo il folder di destinazione
        rhea::fs::folderCreate (glob->usbFolder_Audit);

        char src[256];
        sprintf_s (src, sizeof(src), "%s/dataAudit%d.txt", glob->tempFolder, fileID);

        char dstFilename[64];
        rhea::DateTime dt;
        dt.setNow();
        dt.formatAs_YYYYMMDDHHMMSS (s, sizeof(s), '_', '-', '-');
        sprintf_s (dstFilename, sizeof(dstFilename), "dataAudit_%s.txt", s);

        char dst[256];
        sprintf_s (dst, sizeof(dst), "%s/%s", glob->usbFolder_Audit, dstFilename);
        if (!rhea::fs::fileCopy((const u8*)src, (const u8*)dst))
        {
            priv_pleaseWaitSetError("Error copying file to USB");
        }
        else
        {
            sprintf_s (s, sizeof(s), "Finalizing copy...");
            priv_syncUSBFileSystem(4000);

            sprintf_s (s, sizeof(s), "SUCCESS.<br>The file <b>%s</b> has been copied to your USB pendrive on the folder rhea/rheaDataAudit", dstFilename);
            priv_pleaseWaitSetOK (s);
        }

        priv_pleaseWaitHide();

    }
    else
    {
        dwnloadDataAuditCallBack = eDwnloadDataAuditCallBack_none;
        sprintf_s (s, sizeof(s), "Downloading data audit... ERROR: %s", rhea::app::utils::verbose_readDataFileStatus(status));
        priv_pleaseWaitSetError(s);
        priv_pleaseWaitHide();
    }
}



/************************************************************+
 * Download Diagnostic
 */
void FormBoot::on_btnDownload_diagnostic_clicked()
{
    char s[512];
    priv_pleaseWaitShow("Preparing service zip file (it may takes up to 2 minutes)...");

    sprintf_s (s, sizeof(s), "%s/RHEA_ServicePack.tar.gz", rhea::getPhysicalPathToAppFolder());
    rhea::fs::fileDelete((const u8*)s);

    //download del data audit. Al termine del download, la procedura riprende con la fn priv_on_btnDownload_diagnostic_makeZip()
    priv_startDownloadDataAudit (eDwnloadDataAuditCallBack_service);
}

void FormBoot::priv_on_btnDownload_diagnostic_downloadDataAudit (rhea::thread::sMsg &msg)
{
    cpubridge::eReadDataFileStatus status;
    u16 totKbSoFar = 0;
    u16 fileID = 0;
    cpubridge::translateNotify_READ_DATA_AUDIT_PROGRESS (msg, &status, &totKbSoFar, &fileID);

    char s[512];
    if (status == cpubridge::eReadDataFileStatus_inProgress)
    {
        sprintf_s (s, sizeof(s), "Preparing service zip file (it may takes up to 2 minutes)...<br>Downloading data audit... %d KB", totKbSoFar);
        priv_pleaseWaitSetText (s);
    }
    else if (status == cpubridge::eReadDataFileStatus_finishedOK)
    {
        dwnloadDataAuditCallBack = eDwnloadDataAuditCallBack_none;

        //finito, lo copio in /current (che poi viene zippata)
        char src[256];
        sprintf_s (src, sizeof(src), "%s/dataAudit%d.txt", glob->tempFolder, fileID);

        char dstFilename[64];
        rhea::DateTime dt;
        dt.setNow();
        dt.formatAs_YYYYMMDDHHMMSS (s, sizeof(s), '_', '-', '-');
        sprintf_s (dstFilename, sizeof(dstFilename), "dataAudit_%s.txt", s);

        char dst[256];
        sprintf_s (dst, sizeof(dst), "%s/%s", glob->current, dstFilename);
        rhea::fs::fileCopy((const u8*)src, (const u8*)dst);

        //passo alla fase successiva
        priv_on_btnDownload_diagnostic_makeZip();
    }
    else
    {
        dwnloadDataAuditCallBack = eDwnloadDataAuditCallBack_none;
        //sprintf_s (s, sizeof(s), "Downloading data audit... ERROR: %s", rhea::app::utils::verbose_readDataFileStatus(status));

        //passo alla fase successiva anche se c'è stato un errore nel download del data audit
        priv_on_btnDownload_diagnostic_makeZip();
    }
}

void FormBoot::priv_on_btnDownload_diagnostic_makeZip()
{
    //eseguo lo scprit che zippa tutto quanto
    char s[512];
    sprintf_s (s, sizeof(s), "Preparing service zip file (it may takes up to 2 minutes)...<br>compressing files...");
    priv_pleaseWaitSetText (s);

    sprintf_s (s, sizeof(s), "%s/makeRheaServicePack.sh", rhea::getPhysicalPathToAppFolder());
    system(s);


    //copio lo zip su USB
    priv_pleaseWaitSetText("Copying to USB...");

    char dstFileName[64];
    rhea::DateTime dt;
    dt.setNow();
    dt.formatAs_YYYYMMDDHHMMSS(s, sizeof(s), '_', '-', '-');
    sprintf_s (dstFileName, sizeof(dstFileName), "RHEA_ServicePack_%s.tar.gz", s);

    char dst[256];
    sprintf_s (s, sizeof(s), "%s/RHEA_ServicePack.tar.gz", rhea::getPhysicalPathToAppFolder());
    sprintf_s (dst, sizeof(dst), "%s/%s", glob->usbFolder, dstFileName);

    if (!rhea::fs::fileCopy ((const u8*)s, (const u8*)dst))
        priv_pleaseWaitSetError("ERROR copying file to USB pendrive");
    else
    {
        priv_pleaseWaitSetText("Finalizing copy...");
        priv_syncUSBFileSystem(5000);


        sprintf_s (s, sizeof(s),"SUCCESS.<br>A file named [<b>%s</b>] has been put on your USB pendrive in the folder /rhea.", dstFileName);
        priv_pleaseWaitSetOK(s);
    }
    priv_pleaseWaitHide();
}



/************************************************************+
 * autoupdate handler
 */
bool FormBoot::priv_autoupdate_exists()
{
    ui->labFileFound_cpuFW->setText("CPU: nothing found");
    priv_autoupdate_setText (ui->labStatus_cpuFW, "pending...");

    ui->labFileFound_gui->setText("GUI: nothing found");
    priv_autoupdate_setText (ui->labStatus_gui, "pending...");

    ui->labFileFound_da3->setText("VMC datafile: nothing found");
    priv_autoupdate_setText (ui->labStatus_da3, "pending...");

    memset (autoupdate.cpuFileName, 0, sizeof(autoupdate.cpuFileName));
    memset (autoupdate.da3FileName, 0, sizeof(autoupdate.da3FileName));
    memset (autoupdate.guiFolderName, 0, sizeof(autoupdate.guiFolderName));

    if (!rhea::fs::folderExists(glob->usbFolder_AutoF2))
        return false;

    bool ret = false;
    OSFileFind ff;

    //dentro la cartella, c'è un file per la CPU?
    if (rhea::fs::findFirst(&ff, glob->usbFolder_AutoF2, (const u8*)"*.mhx"))
    {
        do
        {
            if (rhea::fs::findIsDirectory(ff))
                continue;

            ret = true;
            rhea::fs::findGetFileName (ff, (u8*)autoupdate.cpuFileName, sizeof(autoupdate.cpuFileName));
            ui->labFileFound_cpuFW->setText(QString("CPU: ") +autoupdate.cpuFileName);
            break;

        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);
    }

    //dentro la cartella, c'è un file da3?
    if (rhea::fs::findFirst(&ff, glob->usbFolder_AutoF2, (const u8*)"*.da3"))
    {
        do
        {
            if (rhea::fs::findIsDirectory(ff))
                continue;

            ret = true;
            rhea::fs::findGetFileName (ff, (u8*)autoupdate.da3FileName, sizeof(autoupdate.da3FileName));
            ui->labFileFound_da3->setText(QString("VMC datafile: ") +autoupdate.da3FileName);
            break;

        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);
    }

    //dentro la cartella, c'è una cartella con una valida gui
    if (rhea::fs::findFirst(&ff, glob->usbFolder_AutoF2, (const u8*)"*.*"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
                continue;

            const char *dirName = (const char*)rhea::fs::findGetFileName(ff);
            if (dirName[0] == '.')
                continue;

            char s[512];
            sprintf_s (s, sizeof(s), "%s/%s/template.rheagui", glob->usbFolder_AutoF2, dirName);
            if (rhea::fs::fileExists((const u8*)s))
            {
                ret = true;
                rhea::fs::findGetFileName (ff, (u8*)autoupdate.guiFolderName, sizeof(autoupdate.guiFolderName));
                ui->labFileFound_gui->setText(QString("GUI: ") +autoupdate.guiFolderName);
                break;
            }

        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);
    }

    return ret;
}

//**********************************************************************
void FormBoot::priv_autoupdate_setTextWithColor (QLabel *lab, const char *message, const char *bgColor, const char *textColor)
{
    char s[256];
    sprintf_s (s, sizeof(s), "QLabel { background-color:%s; color:%s; }", bgColor, textColor);
    lab->setStyleSheet(s);
    lab->setVisible(true);
    lab->setText(message);
    QApplication::processEvents();
}

void FormBoot::priv_autoupdate_setText (QLabel *lab, const char *message)               { priv_autoupdate_setTextWithColor (lab, message, "#9b9", "#fff"); }
void FormBoot::priv_autoupdate_setOK (QLabel *lab, const char *message)                 { priv_autoupdate_setTextWithColor (lab, message, "#43b441", "#fff"); }
void FormBoot::priv_autoupdate_setError (QLabel *lab, const char *message)              { priv_autoupdate_setTextWithColor (lab, message, "#f00", "#fff"); }

void FormBoot::priv_autoupdate_center (QLabel *lab)
{
    int x = (ui->frameAutoUpdate->width() - lab->width()) / 2;
    lab->move (x, lab->y());
}

void FormBoot::priv_autoupdate_toTheLeft (QLabel *lab)
{
    lab->move (10, lab->y());
}


void FormBoot::priv_autoupdate_showForm()
{
    autoupdate.isRunning = true;
    autoupdate.fase = eAutoUpdateFase_begin;
    autoupdate.skipInHowManySec = 7;
    autoupdate.nextTimeTickMSec = 0;

    ui->labFirstMessage->setVisible(true);
    ui->btnSkipAll->setText ("SKIP");
    ui->btnSkipAll->setVisible(true);

    priv_autoupdate_center(ui->labFileFound_cpuFW);
    priv_autoupdate_center(ui->labStatus_cpuFW);
    ui->btnSkipCPU->setVisible(false);

    priv_autoupdate_center(ui->labFileFound_gui);
    priv_autoupdate_center(ui->labStatus_gui);
    ui->btnSkipGUI->setVisible(false);

    priv_autoupdate_center(ui->labFileFound_da3);
    priv_autoupdate_center(ui->labStatus_da3);
    ui->btnSkipDA3->setVisible(false);

    ui->labFinalMessage->setVisible(false);

    ui->frameAutoUpdate->move(10, 35);
    ui->frameAutoUpdate->setVisible(true);
    ui->frameAutoUpdate->raise();

}

//*****************************************************
void FormBoot::priv_autoupdate_onTick()
{
    const u64 timeNowMSec = rhea::getTimeNowMSec();
    if (timeNowMSec < autoupdate.nextTimeTickMSec)
        return;
    autoupdate.nextTimeTickMSec = timeNowMSec+1000;

    char s[512];

    switch (autoupdate.fase)
    {
    case eAutoUpdateFase_begin:
            //ho appena mostrato il form, sto visualizzando il msg iniziale ed il btn SKIP
            if (autoupdate.skipInHowManySec == 0)
            {
                ui->btnSkipAll->setVisible(false);
                autoupdate.fase = eAutoUpdateFase_cpu_start;
            }
            else
            {
                sprintf_s (s, sizeof(s), "SKIP (%d)", autoupdate.skipInHowManySec);
                ui->btnSkipAll->setText (s);
                autoupdate.skipInHowManySec--;
            }
        break;


    case eAutoUpdateFase_cpu_start:
        if (autoupdate.cpuFileName[0] == 0x00)
        {
            priv_autoupdate_setOK (ui->labStatus_cpuFW, "skipped, no valid file found");
            autoupdate.fase = eAutoUpdateFase_gui_start;
        }
        else
        {
            priv_autoupdate_toTheLeft(ui->labFileFound_cpuFW);
            priv_autoupdate_toTheLeft(ui->labStatus_cpuFW);
            ui->btnSkipCPU->setText ("SKIP");
            ui->btnSkipCPU->setVisible(true);
            autoupdate.fase = eAutoUpdateFase_cpu_waitForSkip;
            autoupdate.skipInHowManySec = 5;
        }
        break;

    case eAutoUpdateFase_cpu_waitForSkip:
        if (autoupdate.skipInHowManySec == 0)
        {
            ui->btnSkipCPU->setVisible(false);
            autoupdate.fase = eAutoUpdateFase_cpu_upload;
        }
        else
        {
            sprintf_s (s, sizeof(s), "SKIP (%d)", autoupdate.skipInHowManySec);
            ui->btnSkipCPU->setText (s);
            autoupdate.skipInHowManySec--;
        }
    break;

    case eAutoUpdateFase_cpu_upload:
        autoupdate.fase = eAutoUpdateFase_cpu_upload_wait;
        priv_autoupdate_setText(ui->labStatus_cpuFW, "Installing CPU FW...");
        sprintf_s (s, sizeof(s), "%s/%s", glob->usbFolder_AutoF2, autoupdate.cpuFileName);
        priv_startUploadCPUFW (eUploadCPUFWCallBack_auto, (const u8*)s);
        break;

    case eAutoUpdateFase_cpu_upload_wait:
        break;

    case eAutoUpdateFase_cpu_upload_finishedKO:
        autoupdate.fase = eAutoUpdateFase_finishedKO;
        break;

    case eAutoUpdateFase_cpu_upload_finishedOK:
        autoupdate.fase = eAutoUpdateFase_gui_start;
        break;





    case eAutoUpdateFase_gui_start:
        priv_autoupdate_center(ui->labFileFound_cpuFW);
        priv_autoupdate_center(ui->labStatus_cpuFW);
        if (autoupdate.guiFolderName[0] == 0x00)
        {
            priv_autoupdate_setOK (ui->labStatus_gui, "skipped, no valid file found");
            autoupdate.fase = eAutoUpdateFase_da3_start;
        }
        else
        {
            priv_autoupdate_toTheLeft(ui->labFileFound_gui);
            priv_autoupdate_toTheLeft(ui->labStatus_gui);
            ui->btnSkipGUI->setText ("SKIP");
            ui->btnSkipGUI->setVisible(true);
            autoupdate.fase = eAutoUpdateFase_gui_waitForSkip;
            autoupdate.skipInHowManySec = 5;
        }
        break;

    case eAutoUpdateFase_gui_waitForSkip:
        if (autoupdate.skipInHowManySec == 0)
        {
            ui->btnSkipGUI->setVisible(false);
            autoupdate.fase = eAutoUpdateFase_gui_upload;
        }
        else
        {
            sprintf_s (s, sizeof(s), "SKIP (%d)", autoupdate.skipInHowManySec);
            ui->btnSkipGUI->setText (s);
            autoupdate.skipInHowManySec--;
        }
    break;

    case eAutoUpdateFase_gui_upload:
        autoupdate.fase = eAutoUpdateFase_gui_upload_copy;
        priv_autoupdate_setText(ui->labStatus_gui, "Installing GUI, please wait...");
        break;

    case eAutoUpdateFase_gui_upload_copy:
        sprintf_s (s, sizeof(s), "%s/%s", glob->usbFolder_AutoF2, autoupdate.guiFolderName);
        if (priv_doInstallGUI ((const u8*)s))
        {
            priv_autoupdate_setOK (ui->labStatus_gui, "SUCCESS, GUI installed");
            autoupdate.fase = eAutoUpdateFase_da3_start;
        }
        else
        {
            priv_autoupdate_setError (ui->labStatus_gui, "ERROR copying files");
            autoupdate.fase = eAutoUpdateFase_finishedKO;
        }
        break;








    case eAutoUpdateFase_da3_start:
        priv_autoupdate_center(ui->labFileFound_gui);
        priv_autoupdate_center(ui->labStatus_gui);
        if (autoupdate.da3FileName[0] == 0x00)
        {
            priv_autoupdate_setOK (ui->labStatus_da3, "skipped, no valid file found");
            autoupdate.fase = eAutoUpdateFase_finished;
        }
        else
        {
            priv_autoupdate_toTheLeft(ui->labFileFound_da3);
            priv_autoupdate_toTheLeft(ui->labStatus_da3);
            ui->btnSkipDA3->setText ("SKIP");
            ui->btnSkipDA3->setVisible(true);
            autoupdate.fase = eAutoUpdateFase_da3_waitForSkip;
            autoupdate.skipInHowManySec = 5;
        }
        break;

    case eAutoUpdateFase_da3_waitForSkip:
        if (autoupdate.skipInHowManySec == 0)
        {
            ui->btnSkipDA3->setVisible(false);
            autoupdate.fase = eAutoUpdateFase_da3_upload;
        }
        else
        {
            sprintf_s (s, sizeof(s), "SKIP (%d)", autoupdate.skipInHowManySec);
            ui->btnSkipDA3->setText (s);
            autoupdate.skipInHowManySec--;
        }
    break;

    case eAutoUpdateFase_da3_upload:
        priv_autoupdate_setText (ui->labStatus_da3, "Installing VMC Settings...");
        sprintf_s (s, sizeof(s), "%s/%s", glob->usbFolder_AutoF2, autoupdate.da3FileName);
        priv_startUploadDA3 (eUploadDA3CallBack_auto, (const u8*)s);
        autoupdate.fase = eAutoUpdateFase_da3_upload_wait;
        break;

    case eAutoUpdateFase_da3_upload_wait:
        break;

    case eAutoUpdateFase_da3_upload_finishedOK:
        autoupdate.fase = eAutoUpdateFase_finished;
        break;

    case eAutoUpdateFase_da3_upload_finishedKO:
        autoupdate.fase = eAutoUpdateFase_finishedKO;
        break;





    case eAutoUpdateFase_finished:
        autoupdate.fase = eAutoUpdateFase_finished_wait;
        priv_autoupdate_center(ui->labFileFound_da3);
        priv_autoupdate_center(ui->labStatus_da3);
        priv_autoupdate_setOK (ui->labFinalMessage, "Auto update finished. Please restart the machine");
        break;

    case eAutoUpdateFase_finishedKO:
        autoupdate.fase = eAutoUpdateFase_finished_wait;
        priv_autoupdate_center(ui->labFileFound_da3);
        priv_autoupdate_center(ui->labStatus_da3);
        priv_autoupdate_setError (ui->labFinalMessage, "Auto update failed. Please restart the machine");
        break;

    case eAutoUpdateFase_finished_wait:
        break;

    case eAutoUpdateFase_backToFormBoot:
        autoupdate.isRunning = false;
        ui->frameAutoUpdate->setVisible(false);
        break;
    }
}

//*****************************************************
void FormBoot::on_btnSkipAll_clicked()
{
    ui->btnSkipAll->setVisible(false);
    autoupdate.fase = eAutoUpdateFase_backToFormBoot;
}

void FormBoot::on_btnSkipCPU_clicked()
{
    ui->btnSkipCPU->setVisible(false);
    priv_autoupdate_setOK (ui->labStatus_cpuFW, "skipped");
    autoupdate.fase = eAutoUpdateFase_gui_start;

}

void FormBoot::on_btnSkipGUI_clicked()
{
    ui->btnSkipGUI->setVisible(false);
    priv_autoupdate_setOK (ui->labStatus_gui, "skipped");
    autoupdate.fase = eAutoUpdateFase_da3_start;
}


void FormBoot::on_btnSkipDA3_clicked()
{
    ui->btnSkipDA3->setVisible(false);
    priv_autoupdate_setOK (ui->labStatus_da3, "skipped");
    autoupdate.fase = eAutoUpdateFase_finished;
}

//******************************************************************
void FormBoot::priv_autoupdate_onCPU_upload (rhea::thread::sMsg &msg)
{
    cpubridge::eWriteCPUFWFileStatus status;
    u16 param = 0;
    cpubridge::translateNotify_WRITE_CPUFW_PROGRESS (msg, &status, &param);

    char s[512];
    if (status == cpubridge::eWriteCPUFWFileStatus_inProgress_erasingFlash)
    {
        priv_autoupdate_setText(ui->labStatus_cpuFW, "Installing CPU FW...erasing flash");
    }
    else if (status == cpubridge::eWriteCPUFWFileStatus_inProgress)
    {
        sprintf_s (s, sizeof(s), "Installing CPU FW... %d KB", param);
        priv_autoupdate_setText(ui->labStatus_cpuFW, s);
    }
    else if (status == cpubridge::eWriteCPUFWFileStatus_finishedOK)
    {
        upldCPUFWCallBack = eUploadCPUFWCallBack_none;
        sprintf_s (s, sizeof(s), "Installing CPU FW... SUCCESS");
        priv_autoupdate_setOK(ui->labStatus_cpuFW, s);
        priv_updateLabelInfo();
        autoupdate.fase = eAutoUpdateFase_cpu_upload_finishedOK;
    }
    else
    {
        upldCPUFWCallBack = eUploadCPUFWCallBack_none;
        sprintf_s (s, sizeof(s), "Installing CPU FW... ERROR: %s [%d]", rhea::app::utils::verbose_WriteCPUFWFileStatus(status), param);
        priv_autoupdate_setError(ui->labStatus_cpuFW, s);
        priv_updateLabelInfo();
        autoupdate.fase = eAutoUpdateFase_cpu_upload_finishedKO;
    }
}

//******************************************************************
void FormBoot::priv_autoupdate_onDA3_upload (rhea::thread::sMsg &msg)
{
    cpubridge::eWriteDataFileStatus status;
    u16 totKbSoFar = 0;
    cpubridge::translateNotify_WRITE_VMCDATAFILE_PROGRESS (msg, &status, &totKbSoFar);

    char s[512];
    if (status == cpubridge::eWriteDataFileStatus_inProgress)
    {
        sprintf_s (s, sizeof(s), "Installing VMC Settings...... %d KB", totKbSoFar);
        priv_autoupdate_setText (ui->labStatus_da3, s);
    }
    else if (status == cpubridge::eWriteDataFileStatus_finishedOK)
    {
        upldDA3CallBack = eUploadDA3CallBack_none;
        sprintf_s (s, sizeof(s), "Installing VMC Settings...... SUCCESS");
        priv_autoupdate_setOK (ui->labStatus_da3, s);
        priv_updateLabelInfo();
        autoupdate.fase = eAutoUpdateFase_da3_upload_finishedOK;
    }
    else
    {
        upldDA3CallBack = eUploadDA3CallBack_none;
        sprintf_s (s, sizeof(s), "Installing VMC Settings...... ERROR: %s", rhea::app::utils::verbose_writeDataFileStatus(status));
        priv_autoupdate_setError (ui->labStatus_da3, s);
        priv_updateLabelInfo();
        autoupdate.fase = eAutoUpdateFase_da3_upload_finishedKO;
    }

}

