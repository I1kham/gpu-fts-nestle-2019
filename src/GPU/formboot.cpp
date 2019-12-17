#include "header.h"
#include "formboot.h"
#include "ui_formboot.h"
#include "history.h"
#include "../rheaAppLib/rheaAppUtils.h"


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
    bBtnStartVMCEnabled = true;

    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);


    ui->frameFileList->setVisible(false);

    ui->labWait->setVisible(false);

    //CPU message/status
    ui->labCPUMessage->setText("");
    ui->labCPUStatus->setText ("");

    //Software version
    ui->labVersion_CPU->setText("");
    ui->labVersion_GPU->setText("");
    ui->labVersion_protocol->setText("");
    ui->labGPU_buildDate->setText ("Build date: " __DATE__ " " __TIME__);


    //Bottoni
    ui->btnInstall_languages->setVisible(false);

    ui->framePleaseWait->setVisible(false);
    ui->framePleaseWait->move (0, 250);

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

    priv_enableButton(ui->btnDownload_audit, false);
    if (glob->bSyncWithCPUResult)
    {
        cpubridge::ask_CPU_QUERY_INI_PARAM(glob->subscriber, 0);
        cpubridge::ask_CPU_QUERY_STATE(glob->subscriber, 0);
    }
    else
    {
        //ci sono stati dei problemi con la sincronizzazione con la CPU.
        //In generale questo vuol dire che la CPU non è installata oppure è una versione non compatibile.
        //Disbailito il pulsante di START VMC e chiedo di aggiornare il FW
        foreverDisableBtnStartVMC();
        priv_pleaseWaitSetError("WARNING: There was an error during synchronization with the CPU.<br>Please upgrade the CPU FW to a compatible version.");
    }
    priv_updateLabelInfo();
    this->show();
}

//*******************************************
eRetCode FormBoot::onTick()
{
    if (retCode != eRetCode_none)
        return retCode;

    //vediamo se CPUBridge ha qualcosa da dirmi
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg(glob->subscriber.hFromCpuToOtherR, &msg))
    {
        priv_onCPUBridgeNotification(msg);
        rhea::thread::deleteMsg(msg);
    }

    return eRetCode_none;
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
    if (rhea::fs::findFirst (&ff, glob->last_installed_cpu, "*.mhx"))
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
    if (rhea::fs::findFirst (&ff, glob->last_installed_da3, "*.da3"))
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
    if (rhea::fs::findFirst (&ff, glob->last_installed_gui, "*.rheagui"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
            {
                char onlyFileName[256];
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
    if (rhea::fs::findFirst (&ff, glob->last_installed_manual, "*.*"))
    {
        do
        {
            if (rhea::fs::findIsDirectory(ff))
            {
                const char *folderName = rhea::fs::findGetFileName(ff);
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
    rhea::fs::fileDelete(s);

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

    rhea::fs::fileDelete(s);
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
                priv_enableButton (ui->btnDownload_audit, false);
            else
            {
                glob->bCPUEnteredInMainLoop=1;
                priv_enableButton (ui->btnDownload_audit, true);
            }

            //non dovrebbe mai succede che la CPU vada da sola in PROG, ma se succede io faccio apparire il vecchio menu PROG
            if (vmcState == cpubridge::eVMCState_PROGRAMMAZIONE)
                retCode = eRetCode_gotoFormOldMenuProg;
            //questo è il caso in cui la CPU non ha portato a termine un LAV SANITARIO. Spegnendo e riaccendendo la macchina, la
            //CPU va da sola in LAV_SANITARIO e io di conseguenza devo andare nel nuovo menu prog alla pagina corretta
            else if (vmcState == cpubridge::eVMCState_LAVAGGIO_SANITARIO)
                retCode = eRetCode_gotoNewMenuProg_LavaggioSanitario;
        }
        break;

    case CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE:
        {
            cpubridge::sCPULCDMessage cpuMsg;
            cpubridge::translateNotify_CPU_NEW_LCD_MESSAGE(msg, &cpuMsg);

            u32 i=0;
            for (; i< cpuMsg.ct; i++)
                msgCPU[i] = cpuMsg.buffer[i];
            msgCPU[i] = 0;
            ui->labCPUMessage->setText(QString(msgCPU, -1));
        }
        break;

    case CPUBRIDGE_NOTIFY_BTN_PROG_PRESSED:
        //l'utente ha premuto il btn PROG, devo andare in programmazione
        break;

    case CPUBRIDGE_NOTIFY_READ_DATA_AUDIT_PROGRESS:
        {
            cpubridge::eReadDataFileStatus status;
            u16 totKbSoFar = 0;
            u16 fileID = 0;
            cpubridge::translateNotify_READ_DATA_AUDIT_PROGRESS (msg, &status, &totKbSoFar, &fileID);

            char s[512];
            if (status == cpubridge::eReadDataFileStatus_inProgress)
            {
                sprintf_s (s, sizeof(s), "Downloading data audit... %d Kb", totKbSoFar);
                priv_pleaseWaitSetText (s);
            }
            else if (status == cpubridge::eReadDataFileStatus_finishedOK)
            {
                sprintf_s (s, sizeof(s), "Downloading data audit... SUCCESS. Copying to USB folder");
                priv_pleaseWaitSetOK (s);

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
                if (!rhea::fs::fileCopy(src, dst))
                {
                    priv_pleaseWaitSetError("Error copying file to USB");
                }
                else
                {
                    sprintf_s (s, sizeof(s), "Finalizing copy...");
                    priv_syncUSBFileSystem(2000);

                    sprintf_s (s, sizeof(s), "SUCCESS.<br>The file <b>%s</b> has been copied to your USB pendrive on the folder rhea/rheaDataAudit", dstFilename);
                    priv_pleaseWaitSetOK (s);
                }

                priv_pleaseWaitHide();

            }
            else
            {
                sprintf_s (s, sizeof(s), "Downloading data audit... ERROR: %s", rhea::app::utils::verbose_readDataFileStatus(status));
                priv_pleaseWaitSetError(s);
                priv_pleaseWaitHide();
            }

        }
        break;

    case CPUBRIDGE_NOTIFY_WRITE_VMCDATAFILE_PROGRESS:
        {
            cpubridge::eWriteDataFileStatus status;
            u16 totKbSoFar = 0;
            cpubridge::translateNotify_WRITE_VMCDATAFILE_PROGRESS (msg, &status, &totKbSoFar);

            char s[512];
            if (status == cpubridge::eWriteDataFileStatus_inProgress)
            {
                sprintf_s (s, sizeof(s), "Installing VMC Settings...... %d Kb", totKbSoFar);
                priv_pleaseWaitSetText (s);
            }
            else if (status == cpubridge::eWriteDataFileStatus_finishedOK)
            {
                sprintf_s (s, sizeof(s), "Installing VMC Settings...... SUCCESS");
                priv_pleaseWaitSetOK (s);
                priv_pleaseWaitHide();
                priv_updateLabelInfo();
            }
            else
            {
                sprintf_s (s, sizeof(s), "Installing VMC Settings...... ERROR: %s", rhea::app::utils::verbose_writeDataFileStatus(status));
                priv_pleaseWaitSetError(s);
                priv_pleaseWaitHide();
                priv_updateLabelInfo();
            }

        }
        break;

    case CPUBRIDGE_NOTIFY_WRITE_CPUFW_PROGRESS:
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
                sprintf_s (s, sizeof(s), "Installing CPU FW... %d Kb", param);
                priv_pleaseWaitSetText (s);
            }
            else if (status == cpubridge::eWriteCPUFWFileStatus_finishedOK)
            {
                sprintf_s (s, sizeof(s), "Installing CPU FW... SUCCESS, please restart the machine");
                priv_pleaseWaitSetOK (s);
                priv_pleaseWaitHide();
                priv_updateLabelInfo();
            }
            else
            {
                sprintf_s (s, sizeof(s), "Installing CPU FW... ERROR: %s [%d]", rhea::app::utils::verbose_WriteCPUFWFileStatus(status), param);
                priv_pleaseWaitSetError(s);
                priv_pleaseWaitHide();
                priv_updateLabelInfo();
            }

        }
        break;
    }
}



//***********************************************************
void FormBoot::foreverDisableBtnStartVMC()
{
    if (!bBtnStartVMCEnabled)
        return;
    bBtnStartVMCEnabled = false;
    ui->buttonStart->setStyleSheet("QPushButton {color: #FF0000; border:  none}");
    ui->buttonStart->setText ("Please restart\nVMC");
}

//*******************************************
void FormBoot::on_buttonStart_clicked()
{
    if (bBtnStartVMCEnabled == false)
        return;

    //verifico che ci sia una GUI installata
    char s[256];
    sprintf_s (s, sizeof(s), "%s/current/gui/template.rheagui", rhea::getPhysicalPathToAppFolder());
    if (!rhea::fs::fileExists(s))
    {
        priv_pleaseWaitShow("");
        priv_pleaseWaitSetError("ERROR: GUI is missing. Please load a new GUI");
        priv_pleaseWaitHide();
        return;
    }

    priv_pleaseWaitShow("Starting VMC...");

#ifdef PLATFORM_YOCTO_EMBEDDED
    sprintf_s (s, sizeof(s), "umount -f %s", USB_MOUNTPOINT);
    system(s);
#endif

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
void FormBoot::priv_fileListPopulate(const char *pathNoSlash, const char *jolly, bool bClearList)
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
            ui->lbFileList->addItem(rhea::fs::findGetFileName(ff));
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
        priv_uploadDA3(src);
        break;

    case eFileListMode_Manual:
        sprintf_s (src, sizeof(src), "%s/%s", glob->usbFolder_Manual, srcFilename.toStdString().c_str());
        priv_fileListHide();
        priv_uploadManual(src);
        break;

    case eFileListMode_GUI:
        sprintf_s (src, sizeof(src), "%s/%s", glob->usbFolder_GUI, srcFilename.toStdString().c_str());
        priv_fileListHide();
        priv_uploadGUI(src);
        break;

    case eFileListMode_CPU:
        sprintf_s (src, sizeof(src), "%s/%s", glob->usbFolder_CPUFW, srcFilename.toStdString().c_str());
        priv_fileListHide();
        priv_uploadCPUFW(src);
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
bool FormBoot::priv_langCopy (const char *srcFolder, const char *dstFolder, u32 timeToWaitDuringCopyFinalizingMSec)
{
    priv_pleaseWaitShow("copying files...");

    QApplication::processEvents();

    //se la directory dst non esiste, la creo, se esiste, la svuoto
    {
        QDir root_dir(dstFolder);
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
    if (rhea::fs::findFirst(&ff, glob->usbFolder_Manual, "*.*"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
                continue;
            const char *dirName = rhea::fs::findGetFileName(ff);
            if (dirName[0] == '.')
                continue;

            sprintf_s (s, sizeof(s), "%s/%s/index.html", glob->usbFolder_Manual, dirName);
            if (rhea::fs::fileExists(s))
            {
                ui->lbFileList->addItem(dirName);
            }


        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);

        if (ui->lbFileList->count())
            ui->lbFileList->item(0)->setSelected(true);
    }
}

void FormBoot::priv_uploadManual (const char *srcFullFolderPath)
{
    char srcOnlyFolderName[256];
    char s[256];
    rhea::fs::extractFileNameWithoutExt (srcFullFolderPath, srcOnlyFolderName, sizeof(srcOnlyFolderName));
    if (strcmp(srcOnlyFolderName,"REMOVE_MANUAL") == 0)
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


        sprintf_s (s, sizeof(s), "%s/%s", glob->last_installed_manual, srcOnlyFolderName);
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
    priv_fileListShow(eFileListMode_DA3);
    priv_fileListPopulate(glob->usbFolder_VMCSettings, "*.da3", true);
}

void FormBoot::priv_uploadDA3 (const char *fullFilePathAndName)
{
    priv_pleaseWaitShow("Installing VMC Settings...");
    cpubridge::ask_WRITE_VMCDATAFILE (glob->subscriber, 0, fullFilePathAndName);
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
    if (rhea::fs::findFirst(&ff, glob->usbFolder_GUI, "*.*"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
                continue;
            const char *dirName = rhea::fs::findGetFileName(ff);
            if (dirName[0] == '.')
                continue;

            sprintf_s (s, sizeof(s), "%s/%s/template.rheagui", glob->usbFolder_GUI, dirName);
            if (rhea::fs::fileExists(s))
            {
                ui->lbFileList->addItem(dirName);
            }


        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);

        if (ui->lbFileList->count())
            ui->lbFileList->item(0)->setSelected(true);
    }
}

void FormBoot::priv_uploadGUI (const char *srcFullFolderPath)
{
    char s[512];
    priv_pleaseWaitShow("Installing GUI...");

    //elimino la roba attualmente installata
    rhea::fs::deleteAllFileInFolderRecursively(glob->current_GUI, false);
    rhea::fs::deleteAllFileInFolderRecursively (glob->last_installed_gui, false);

    //copio la GUI nella cartella locale
    char srcOnlyFolderName[256];
    rhea::fs::extractFileNameWithoutExt (srcFullFolderPath, srcOnlyFolderName, sizeof(srcOnlyFolderName));
    if (!rhea::fs::folderCopy(srcFullFolderPath, glob->current_GUI))
        priv_pleaseWaitSetError("ERROR copying files");
    else
    {
        //creo un file con il nome del folder e lo salvo in last_installed_gui in modo da poter visualizzare il "nome" dell'ultima gui installata
        sprintf_s (s, sizeof(s), "%s/%s.rheagui", glob->last_installed_gui, srcOnlyFolderName);
        FILE *f = fopen (s, "wt");
        {
            rhea::DateTime dt;
            dt.setNow();
            dt.formatAs_YYYYMMDDHHMMSS(s, sizeof(s), ' ', '/', ':');
            fprintf (f, "%s", s);
        }
        fclose(f);

        priv_pleaseWaitSetOK("SUCCESS.<br>GUI installed");
    }



    priv_pleaseWaitHide();
    priv_updateLabelInfo();
}


/************************************************************+
 * Install CPU FW
 */
void FormBoot::on_btnInstall_CPU_clicked()
{
    priv_fileListShow(eFileListMode_CPU);
    priv_fileListPopulate(glob->usbFolder_CPUFW, "*.mhx", true);

}

void FormBoot::priv_uploadCPUFW (const char *fullFilePathAndName)
{
    priv_pleaseWaitShow("Installing CPU FW...");
    foreverDisableBtnStartVMC();
    cpubridge::ask_WRITE_CPUFW (glob->subscriber, 0, fullFilePathAndName);
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
    if (rhea::fs::findFirst (&ff, glob->last_installed_da3, "*.da3"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
            {
                strcpy (lastInstalledDa3FileName, rhea::fs::findGetFileName(ff));
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
    if (rhea::fs::fileExists(dst))
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
    if (!rhea::fs::fileCopy(src, dst))
    {
        priv_pleaseWaitSetError("Error copying file to USB");
    }
    else
    {
        priv_pleaseWaitSetText("Finalizing copy...");
        priv_syncUSBFileSystem(5000);

        rhea::fs::extractFileNameWithExt(dst, src, sizeof(src));
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
    if (rhea::fs::findFirst (&ff, glob->last_installed_gui, "*.rheagui"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
            {
                rhea::fs::extractFileNameWithoutExt (rhea::fs::findGetFileName(ff), lastInstalledGUIName, sizeof(lastInstalledGUIName));
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
    if (rhea::fs::folderExists(dst))
    {
        rhea::DateTime dt;
        char data[64];
        dt.setNow();
        dt.formatAs_YYYYMMDDHHMMSS (data, sizeof(data), '-', 0x00, 0x00);
        sprintf_s (dst, sizeof(dst), "%s/%s-%s", glob->usbFolder_GUI, data, lastInstalledGUIName);
        rhea::fs::folderCreate(dst);
    }


    char s[512];
    if (!rhea::fs::folderCopy(glob->current_GUI, dst))
    {
        sprintf_s (s, sizeof(s), "ERROR copying files to [%s]", dst);
        priv_pleaseWaitSetError(s);
    }
    else
    {
        priv_pleaseWaitSetText("Finalizing copy...");
        priv_syncUSBFileSystem(10000);

        rhea::fs::extractFileNameWithoutExt(dst, s, sizeof(s));
        sprintf_s (dst, sizeof(dst), "SUCCESS.<br>GUI <b>%s</b> have been copied to your USB pendrive in folder rhea/rheaGUI", s);
        priv_pleaseWaitSetOK(dst);
    }


    priv_pleaseWaitHide();
}


/************************************************************+
 * Download Diagnostic
 */
void FormBoot::on_btnDownload_diagnostic_clicked()
{
    char s[512];
    priv_pleaseWaitShow("Preparing service zip file (it may takes up to 2 minutes)...");

    sprintf_s (s, sizeof(s), "%s/RHEA_ServicePack.tar.gz", rhea::getPhysicalPathToAppFolder());
    rhea::fs::fileDelete(s);

    sprintf_s (s, sizeof(s), "%s/makeRheaServicePack.sh", rhea::getPhysicalPathToAppFolder());
    system(s);


    priv_pleaseWaitSetText("Copying to USB...");

    char dstFileName[64];
    rhea::DateTime dt;
    dt.setNow();
    dt.formatAs_YYYYMMDDHHMMSS(s, sizeof(s), '_', '-', '-');
    sprintf_s (dstFileName, sizeof(dstFileName), "RHEA_ServicePack_%s.tar.gz", s);

    char dst[256];
    sprintf_s (s, sizeof(s), "%s/RHEA_ServicePack.tar.gz", rhea::getPhysicalPathToAppFolder());
    sprintf_s (dst, sizeof(dst), "%s/%s", glob->usbFolder, dstFileName);

    if (!rhea::fs::fileCopy (s, dst))
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


/**********************************************************************
 * Download data audit
 */
void FormBoot::on_btnDownload_audit_clicked()
{
    priv_pleaseWaitShow("Downloading data audit...");
    cpubridge::ask_READ_DATA_AUDIT (glob->subscriber, 0);
}


