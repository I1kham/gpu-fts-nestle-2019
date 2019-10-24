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
    retCode = 0;
    bBtnStartVMCEnabled = true;

    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    qApp->setStyleSheet("QPushButton { border-radius:5px }");

    utils::getRightFontForLanguage (fntButton, 16, "GB");
    utils::getRightFontForLanguage (fnt12, 12, "GB");

    ui->frameFileList->setVisible(false);

    ui->labWait->setVisible(false);
    ui->labWait->setFont(fnt12);

    //CPU message/status
    ui->labCPUMessage->setText("");
    ui->labCPUMessage->setFont(fnt12);

    ui->labCPUStatus->setText ("");
    ui->labCPUStatus->setFont(fnt12);

    //Software version
    ui->labSoftwareVer->setFont(fntButton);
    ui->labVersion_CPU->setText("");
    ui->labVersion_CPU->setFont(fnt12);
    ui->labVersion_GPU->setText("");
    ui->labVersion_GPU->setFont(fnt12);
    ui->labVersion_protocol->setText("");
    ui->labVersion_protocol->setFont(fnt12);


    //installed version
    ui->labInstalledFiles->setFont(fntButton);
    ui->labInstalled_CPU->setFont(fnt12);
    ui->labInstalled_DA3->setFont(fnt12);
    ui->labInstalled_GUI->setFont(fnt12);
    ui->labInstalled_Manual->setFont(fnt12);


    //list box con elenco file
    ui->lbFileList->setFont(fntButton);
    ui->btnOK->setFont(fntButton);
    ui->btnCancel->setFont(fntButton);


    //Bottoni
    ui->btnInstall_CPU->setFont(fntButton);
    ui->btnInstall_DA3->setFont(fntButton);
    ui->btnInstall_GUI->setFont(fntButton);
    ui->btnInstall_manual->setFont(fntButton);
    ui->btnInstall_languages->setFont(fntButton);
    ui->btnInstall_languages->setVisible(false);

    ui->btnDownload_audit->setFont(fntButton);
    ui->btnDownload_DA3->setFont(fntButton);
    ui->btnDownload_diagnostic->setFont(fntButton);
    ui->btnDownload_GUI->setFont(fntButton);

    ui->buttonStart->setFont(fntButton);




    priv_updateLabelInfo();
}

//***********************************************************
FormBoot::~FormBoot()
{
    delete ui;
}

//*******************************************
void FormBoot::showMe()
{
    retCode = 0;
    cpubridge::ask_CPU_QUERY_STATE(glob->subscriber, 0);
    cpubridge::ask_CPU_QUERY_INI_PARAM(glob->subscriber, 0);

    priv_pleaseWaitSetText("");
    priv_pleaseWaitHide();
    this->show();
}

//*******************************************
int FormBoot::onTick()
{
    if (retCode != 0)
        return retCode;

    //vediamo se CPUBridge ha qualcosa da dirmi
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg(glob->subscriber.hFromCpuToOtherR, &msg))
    {
        priv_onCPUBridgeNotification(msg);
        rhea::thread::deleteMsg(msg);
    }

    return 0;
}

//*******************************************
void FormBoot::priv_setButtonStyle (QPushButton *obj, const char *style)
{
    QString s;
    s = QString ("{ border-radius:5px;") + style +QString ("}");
    obj->setStyleSheet (s);
    obj->setFont(fntButton);
}

//*******************************************
void FormBoot::priv_updateLabelInfo()
{
    char s[512];
    OSFileFind ff;

    //GPU Version
    sprintf_s (s, sizeof(s), "<b>GPU</b>: %d.%d.%d", GPU_VERSION_MAJOR, GPU_VERSION_MINOR, GPU_VERSION_BUILD);
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
                sprintf_s (s, sizeof(s), "<b>CPU</b>: %s", rhea::fs::findGetFileName(ff));
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
                sprintf_s (s, sizeof(s), "<b>VMC Settings</b>: %s", rhea::fs::findGetFileName(ff));
                ui->labInstalled_DA3->setText(s);
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
                sprintf_s (s, sizeof(s), "<b>GUI</b>: %s", onlyFileName);
                ui->labInstalled_GUI->setText(s);
                break;
            }
        } while (rhea::fs::findNext(ff));
        rhea::fs::findClose(ff);
    }


    //Installed files: Manual
    ui->labInstalled_Manual->setText("Manual:");
    if (rhea::fs::findFirst (&ff, glob->last_installed_manual, "*.pdf"))
    {
        do
        {
            if (!rhea::fs::findIsDirectory(ff))
            {
                sprintf_s (s, sizeof(s), "<b>Manual</b>: %s", rhea::fs::findGetFileName(ff));
                ui->labInstalled_Manual->setText(s);
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
    case CPUBRIDGE_NOTIFY_CPU_INI_PARAM:
        {
            char s[256];
            cpubridge::sCPUParamIniziali iniParam;
            cpubridge::translateNotify_CPU_INI_PARAM (msg, &iniParam);

            sprintf_s (s, sizeof(s), "<b>CPU</b>: %s", iniParam.CPU_version);
            ui->labVersion_CPU->setText (s);

            sprintf_s (s, sizeof(s), "<b>Protocol ver</b>: %d", iniParam.protocol_version);
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
                priv_pleaseWaitSetText (s);
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
    priv_pleaseWaitShow("Starting VMC...");
    retCode = 1;
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
        sprintf_s (src, sizeof(src), "%s/%s", glob->usbFolder, srcFilename.toStdString().c_str());
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
        priv_pleaseWaitSetText("SUCCESS");
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
    priv_fileListPopulate(glob->usbFolder, "*.pdf", true);
}

void FormBoot::priv_uploadManual (const char *srcFullFilePathAndName)
{
    //copio il pdf nella cartella locale
    char srcFilename[256];
    rhea::fs::extractFileNameWithExt (srcFullFilePathAndName, srcFilename, sizeof(srcFilename));

    char dstFilePathAndName[512];
    sprintf_s (dstFilePathAndName, sizeof(dstFilePathAndName), "%s/%s", glob->last_installed_manual, srcFilename);

    priv_pleaseWaitShow("Copying manual to local folder...");
    if (!rhea::fs::fileCopy (srcFullFilePathAndName, dstFilePathAndName))
        priv_pleaseWaitSetError("ERROR copying files");
    else
        priv_pleaseWaitSetText("SUCCESS. Manual installed");

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
            dt.formatAs_YYYYMMDDHHMMSS(s, sizeof(s), ' ', '/', ':');
            fprintf (f, "%s", s);
        }
        fclose(f);

        priv_pleaseWaitSetText("SUCCESS. GUI installed");
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
    sprintf_s (dst, sizeof(dst), "%s/%s", glob->usbFolder_VMCSettings, lastInstalledDa3FileName);

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
    rhea::fs::folderCreate(dst);


    char s[512];
    if (!rhea::fs::folderCopy(glob->current_GUI, dst))
    {
        sprintf_s (s, sizeof(s), "ERROR copying files to [%s]", dst);
        priv_pleaseWaitSetError(s);
    }
    else
    {
        sprintf_s (s, sizeof(s), "SUCCESS. Files has been copied to yourt USB pendrive in the folder [%s]", dst);
        priv_pleaseWaitSetText(s);
    }


    priv_pleaseWaitHide();
}


/************************************************************+
 * Download Diagnostic
 */
void FormBoot::on_btnDownload_diagnostic_clicked()
{
    priv_pleaseWaitShow("Downloading Diagnostic zip file...");

    priv_pleaseWaitSetError("Downloading Diagnostic zip file... ERROR: feature not implemented yet");
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


