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


//***********************************************************
eHistoryType history_type = HISTORY_TYPE_UNK;
QString history_lastFileName;
void logToHistoryFile()
{
    if (history_type == HISTORY_TYPE_UNK)
        return;
    if (history_lastFileName.length() == 0)
        return;

    History::addEntry (history_type, history_lastFileName.toStdString().c_str());
    history_lastFileName="";
    history_type = HISTORY_TYPE_UNK;
}


//********************************************************
FormBoot::FormBoot(QWidget *parent, sGlobal *glob) :
    QDialog(parent), ui(new Ui::FormBoot)
{
    this->glob = glob;
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    ui->frameFileList->setVisible(false);

    bBtnStartVMCEnabled = true;
    ui->labWait->setVisible(false);
    ui->labCPUMessage->setText("");
    ui->labCPUMessage->setVisible(true);
    ui->labCPUStatus->setText ("");

    ui->labVersion_GPU->setText("");
    ui->labVersion_CPU->setText("");
    ui->labVersion_protocol->setText("");

    priv_updateLabelInfo();






/*
    //vmc settings
    {
        QStringList nameFilter("*.da3");
        QDir directory(utils::getFolder_Usb_VMCSettings());
        QStringList datFilesAndDirectories = directory.entryList(nameFilter);
        ui->listVMCSettingsFiles->addItems(datFilesAndDirectories);
        if(ui->listVMCSettingsFiles->count()!=0)
            ui->listVMCSettingsFiles->item(0)->setSelected(true);
    }


    //CPU FW
    {
        QStringList nameFilterCPU("*.mhx");
        QDir directoryCPU(utils::getFolder_Usb_CPU());
        QStringList cpuFilesAndDirectories = directoryCPU.entryList(nameFilterCPU);
        ui->listCPUFiles->addItems(cpuFilesAndDirectories);
        if(ui->listCPUFiles->count()!=0)
            ui->listCPUFiles->item(0)->setSelected(true);
    }


    //cerco le GUI sulla chiavetta  USB.
    //Aggiungo solo le GUI FusionBetaV1 (ovvero che abbiano il file js/rheaBootstrap.js
    {
        QDir directoryGUI(utils::getFolder_Usb_GUI());
        QStringList guiFilesAndDirectories = directoryGUI.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Time);
        for (int i=0; i<guiFilesAndDirectories.count(); i++)
        {
            QString path = utils::getFolder_Usb_GUI() +"/" +guiFilesAndDirectories[i] +"/web/js/rheaBootstrap.js";
            if (QFile(path).exists())
                ui->listGUIFiles->addItem(guiFilesAndDirectories[i]);
        }
        //ui->listGUIFiles->addItems(guiFilesAndDirectories);
        if(ui->listGUIFiles->count()!=0)
            ui->listGUIFiles->item(0)->setSelected(true);
    }


    //manual
    {
        QDir directoryManual(utils::getFolder_Usb_Manual());
        QStringList ManualFilesAndDirectories = directoryManual.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Time);
        ui->listManualFiles->addItems(ManualFilesAndDirectories);
        if(ui->listManualFiles->count()!=0)
            ui->listManualFiles->item(0)->setSelected(true);
    }


    //languages
    {
        QDir directory(utils::getFolder_Usb_Lang(), "*.lng", QDir::Unsorted, QDir::Files);
        QStringList list = directory.entryList();
        if (list.count() > 0)
            ui->btnWriteLang->setEnabled(true);
        else
            ui->btnWriteLang->setEnabled(false);

    }
*/

    isInterruptActive = false;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerInterrupt()));
    timer->start (100);
}

//***********************************************************
FormBoot::~FormBoot()
{
    timer->stop();
    delete ui;
}

//*******************************************
void FormBoot::timerInterrupt()
{
    if (isInterruptActive)
        return;

    isInterruptActive=true;

    //vediamo se CPUBridge ha qualcosa da dirmi
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg(glob->subscriber.hFromCpuToOtherR, &msg))
    {
        priv_onCPUBridgeNotification(msg);
        rhea::thread::deleteMsg(msg);
    }

    isInterruptActive=false;
}


//*******************************************
void FormBoot::priv_updateLabelInfo()
{
    char s[512];
    OSFileFind ff;

    //GPU Version
    sprintf_s (s, sizeof(s), "GPU: %d.%d.%d", GPU_VERSION_MAJOR, GPU_VERSION_MINOR, GPU_VERSION_BUILD);
    ui->labVersion_GPU->setText(s);

    //CPU version + protocol version sono aggiornate on the fly mano mano che si ricevono i messaggi da CPU

    //Installed files: CPU
    ui->labInstalled_CPU->setText("CPU:");
    if (rhea::fs::findFirst (&ff, glob->last_installed_cpu, "*.mhx"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
            {
                sprintf_s (s, sizeof(s), "CPU: %s", rhea::fs::findGetFileName(ff));
                ui->labInstalled_CPU->setText(s);
                break;
            }
        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);
    }

    //Installed files: DA3
    ui->labInstalled_DA3->setText("VMC Settings:");
    if (rhea::fs::findFirst (&ff, glob->last_installed_da3, "*.da3"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
            {
                sprintf_s (s, sizeof(s), "VMC Settings: %s", rhea::fs::findGetFileName(ff));
                ui->labInstalled_DA3->setText(s);
                break;
            }
        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);
    }

    //Installed files: GUI
    ui->labInstalled_GUI->setText("GUI:");
    if (rhea::fs::findFirst (&ff, glob->last_installed_gui, "*.gui"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
            {
                sprintf_s (s, sizeof(s), "GUI: %s", rhea::fs::findGetFileName(ff));
                ui->labInstalled_GUI->setText(s);
                break;
            }
        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);
    }
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
        /*
        case CPUBRIDGE_NOTIFY_DYING:
        case CPUBRIDGE_NOTIFY_CPU_CREDIT_CHANGED:
        case CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED:
        case CPUBRIDGE_NOTIFY_CPU_SEL_PRICES_CHANGED:
        case CPUBRIDGE_NOTIFY_CPU_FULLSTATE:
        */

    case CPUBRIDGE_NOTIFY_CPU_INI_PARAM:
        {
            char s[256];
            cpubridge::sCPUParamIniziali iniParam;
            cpubridge::translateNotify_CPU_INI_PARAM (msg, &iniParam);

            sprintf_s (s, sizeof(s), "CPU: %s", iniParam.CPU_version);
            ui->labVersion_CPU->setText (s);

            sprintf_s (s, sizeof(s), "Protocol ver: %d", iniParam.protocol_version);
            ui->labVersion_protocol->setText(s);

            strcpy (glob->cpuVersion, iniParam.CPU_version);
        }
        break;

    case CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED:
        {
            cpubridge::eVMCState vmcState;
            u8 vmcErrorCode, vmcErrorType;
            cpubridge::translateNotify_CPU_STATE_CHANGED (msg, &vmcState, &vmcErrorCode, &vmcErrorType);
            ui->labCPUStatus->setText (rhea::app::utils::verbose_eVMCState (vmcState));
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
                if (!rhea::fs::fileCopy(src, dst))
                {
                    priv_pleaseWaitSetError("Error copying file to USB");
                }
                else
                {
                    sprintf_s (s, sizeof(s), "Finalizing copy...");
                    utils::waitAndProcessEvent(2000);

                    sprintf_s (s, sizeof(s), "SUCCESS. The file [%s] has been copied to your USB pendrive on the folder rhea/rheaDataAudit", dstFilename);
                    priv_pleaseWaitSetText (s);
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
                priv_pleaseWaitSetText (s);
                priv_updateLabelInfo();
                priv_pleaseWaitHide();

            }
            else
            {
                sprintf_s (s, sizeof(s), "Installing VMC Settings...... ERROR: %s", rhea::app::utils::verbose_writeDataFileStatus(status));
                priv_pleaseWaitSetError(s);
                priv_pleaseWaitHide();
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
    priv_pleaseWaitShow("Starting VMC...");
    this->done(0);
}


//**********************************************************************
void FormBoot::priv_pleaseWaitShow(const char *message)
{
    ui->frameInstallBtn->setVisible(false);
    ui->frameDownloadBtn->setVisible(false);
    ui->buttonStart->setVisible(false);
    priv_pleaseWaitSetText(message);
}

//**********************************************************************
void FormBoot::priv_pleaseWaitHide ()
{
    ui->frameInstallBtn->setVisible(true);
    ui->frameDownloadBtn->setVisible(true);
    ui->buttonStart->setVisible(true);
}

//**********************************************************************
void FormBoot::priv_pleaseWaitSetText (const char *message)
{
    if (NULL == message)
        ui->labWait->setVisible(false);
    else if (message[0] == 0x00)
        ui->labWait->setVisible(false);
    else
    {
        ui->labWait->setStyleSheet("QLabel { background-color:#43b441; color:#fff }");
        ui->labWait->setVisible(true);
        ui->labWait->setText(message);
    }
    QApplication::processEvents();
}

//**********************************************************************
void FormBoot::priv_pleaseWaitSetError (const char *message)
{
    if (NULL == message)
        ui->labWait->setVisible(false);
    else if (message[0] == 0x00)
        ui->labWait->setVisible(false);
    else
    {
        ui->labWait->setStyleSheet("QLabel { background-color:#f00; color:#fff }");
        ui->labWait->setVisible(true);
        ui->labWait->setText(message);
    }
    QApplication::processEvents();
}






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
    }
}

void FormBoot::on_lbFileList_doubleClicked(const QModelIndex &index)                { on_btnOK_clicked(); }
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

    //eFileListMode_CPU
    //eFileListMode_GUI
    //eFileListMode_Manual
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
        priv_pleaseWaitSetText("SUCCESS");
    }
    else
        priv_pleaseWaitSetError("Error during file copy!");

    priv_pleaseWaitHide();
    return (ret);
}





//**********************************************************************
void FormBoot::on_btnDownload_audit_clicked()
{
    priv_pleaseWaitShow("Downloading data audit...");
    cpubridge::ask_READ_DATA_AUDIT (glob->subscriber, 0);
}



//**********************************************************************
void FormBoot::on_btnInstall_manual_clicked()
{
    priv_pleaseWaitShow("");
    priv_pleaseWaitSetError("Feature not implemented");
    priv_pleaseWaitHide();
}



//**********************************************************************
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

//**********************************************************************
void FormBoot::on_btnDownload_DA3_clicked()
{
    priv_pleaseWaitShow("Downloading VMC Settings...");

    char src[512];
    sprintf_s (src, sizeof(src), "%s/vmcDataFile.da3", glob->current_da3);


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
    sprintf_s (src, sizeof(src), "%s/%s", glob->usbFolder_VMCSettings, lastInstalledDa3FileName);

    if (rhea::fs::fileExists(dst))
        rhea::fs::fileDelete(dst);

    if (!rhea::fs::fileCopy(src, dst))
    {
        priv_pleaseWaitSetError("Error copying file to USB");
    }
    else
    {
        sprintf_s (dst, sizeof(dst), "SUCCESS. The file [%s] has been copied to your USB pendrive on the folder rhea/rheaData", lastInstalledDa3FileName);
        priv_pleaseWaitSetText (dst);
    }

    priv_pleaseWaitHide();
}












