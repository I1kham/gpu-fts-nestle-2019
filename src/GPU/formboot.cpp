#include "header.h"
#include "formboot.h"
#include "ui_formboot.h"
#include "history.h"

#include <QWidget>
#include <QDir>
#include <QFile>
#include <QTime>
#include <QTimer>
#include <QMessageBox>
#include <QSerialPort>
#include <QThread>
#include <QProcess>
#include <unistd.h>


QString Folder_PenDrive = "/run/media/sda1/rhea";
QString Folder_VMCSettingsPenDrive = "/run/media/sda1/rhea/rheaData";
QString Folder_FirmwareCPUPenDrive = "/run/media/sda1/rhea/rheaFirmwareCPU01";
QString Folder_GUIPenDrive = "/run/media/sda1/rhea/rheaGUI";
QString Folder_ManualPenDrive = "/run/media/sda1/rhea/rheaManual";
QString Folder_AuditPenDrive = "/run/media/sda1/rhea/rheaDataAudit";
QString Folder_LangPenDrive = "/run/media/sda1/rhea/lang";

QTimer *timerBoot;
QString destFilePath;

eHistoryType history_type = HISTORY_TYPE_UNK;
QString history_lastFileName;



int ReadConfigFile_saveToFile(void);
int Audit_saveToFile(void);
unsigned char Serial_WaitCharTimeout(unsigned char datoWait, unsigned long timeoutLoopVal);
void Serial_PutChar(unsigned char datoSend);
void RestartCPU(void);
unsigned char CPU_ConvertToNum(int index_p);
unsigned char WriteByteMasterNext(unsigned char dato_8, unsigned char isLastFlag);
static bool copyRecursively(const QString &srcFilePath, const QString &tgtFilePath);
void myDelay(long msec_p);




extern QSerialPort* serialCPU;
extern unsigned char TxBufCPU[MaxLenBufferComCPU_Tx];
extern QByteArray QarrayTX;
extern int FormStatus;
extern unsigned char FormBoot_ActiveStatus;
extern unsigned char ComCommandRequest;
extern unsigned char ComStatus;
extern QByteArray myFileArray;
extern QByteArray myAuditArray;
extern int myFileArray_index;
extern unsigned char ConfigFileOperation_status;
extern unsigned char ConfigFileOperation_errorCode;
extern QString Folder_VMCSettings;
extern QString Folder_FirmwareCPU;
extern QString Folder_GUI;
extern QString Folder_Manual;
extern QString Folder_languages;

#define labelStatus_H               40
#define labelStatus_W               600

FormBoot::FormBoot(QWidget *parent) :    QDialog(parent), ui(new Ui::FormBoot)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);


    bBtnStartVMCEnabled = true;


    ui->labelStatus->setGeometry((ScreenW-labelStatus_W)/2 ,ScreenH-labelStatus_H-labelStatus_MarginBottom, labelStatus_W, labelStatus_H);
    ui->labelStatus->raise();
    ui->labelStatus->setText("");
    ui->labelStatus->setAlignment(Qt::AlignCenter);

    ui->labelWarning->setVisible(false);


    QStringList nameFilter("*.da3");
    QDir directory(Folder_VMCSettingsPenDrive);
    QStringList datFilesAndDirectories = directory.entryList(nameFilter);
    ui->listVMCSettingsFiles->addItems(datFilesAndDirectories);
    if(ui->listVMCSettingsFiles->count()!=0)
    {
        ui->listVMCSettingsFiles->item(0)->setSelected(true);
    }


    QStringList nameFilterCPU("*.mhx");
    QDir directoryCPU(Folder_FirmwareCPUPenDrive);
    QStringList cpuFilesAndDirectories = directoryCPU.entryList(nameFilterCPU);
    ui->listCPUFiles->addItems(cpuFilesAndDirectories);
    if(ui->listCPUFiles->count()!=0)
    {
        ui->listCPUFiles->item(0)->setSelected(true);
    }



    QDir directoryGUI(Folder_GUIPenDrive);
    QStringList guiFilesAndDirectories = directoryGUI.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Time);
    ui->listGUIFiles->addItems(guiFilesAndDirectories);
    if(ui->listGUIFiles->count()!=0)
    {
        ui->listGUIFiles->item(0)->setSelected(true);
    }


    QDir directoryManual(Folder_ManualPenDrive);
    QStringList ManualFilesAndDirectories = directoryManual.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Time);
    ui->listManualFiles->addItems(ManualFilesAndDirectories);
    if(ui->listManualFiles->count()!=0)
    {
        ui->listManualFiles->item(0)->setSelected(true);
    }


    //languages
    {
        QDir directory(Folder_LangPenDrive, "*.lng", QDir::Unsorted, QDir::Files);
        QStringList list = directory.entryList();
        if (list.count() > 0)
            ui->btnWriteLang->setEnabled(true);
        else
            ui->btnWriteLang->setEnabled(false);

    }

    ui->tabWidget->setCurrentWidget(ui->tabVMCSettings);

    timerBoot = new QTimer(this);
    connect(timerBoot, SIGNAL(timeout()), this, SLOT(timerInterrupt()));

}


FormBoot::~FormBoot()
{
    timerBoot->stop();
    delete ui;
}



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


unsigned char RecType, NumByte, dato8;
int SerialIndex;
#define max_check_cont 400


void FormBoot::foreverDisableBtnStartVMC()
{
    if (!bBtnStartVMCEnabled)
        return;

    bBtnStartVMCEnabled = false;
    ui->buttonStart->setStyleSheet("QPushButton {color: #FF0000; border:  none}");
    ui->buttonStart->setText ("Please restart\nVMC");
}

void FormBoot::timerInterrupt()
{
    int i;
    switch(ConfigFileOperation_status)
    {
        case ConfigFileOperation_status_Write_inProgress:
            this->ui->labelStatus->setText ("Writing: " +QString::number(TxBufCPU[3]) +" / " +QString::number(TxBufCPU[4]));
            break;

        case ConfigFileOperation_status_Write_endKO:
            timerBoot->stop();
            ui->buttonStart->setVisible(true);
            ui->labelStatus->setText("Configuration file write failed " + QString::number(ConfigFileOperation_errorCode));
            ConfigFileOperation_status = ConfigFileOperation_status_idle;
            break;

        case ConfigFileOperation_status_Write_endOK:
            timerBoot->stop();
            foreverDisableBtnStartVMC();
            ui->buttonStart->setVisible(true);
            ui->labelStatus->setText("Configuration file write OK");
            ConfigFileOperation_status = ConfigFileOperation_status_idle;
            logToHistoryFile();
            break;

        case ConfigFileOperation_status_Read_inProgress:
            this->ui->labelStatus->setText ("Reading: " +QString::number(TxBufCPU[3]) +" / " +QString::number(ConfigFileSize/ConfigFile_blockDim));
            break;

        case ConfigFileOperation_status_Read_endKO:
            timerBoot->stop();
            ui->labelStatus->setText("Configuration file read failed " + QString::number(ConfigFileOperation_errorCode));
            ConfigFileOperation_status=ConfigFileOperation_status_idle;
            break;

        case ConfigFileOperation_status_Read_endOK:
            timerBoot->stop();
            ReadConfigFile_saveToFile();
            ui->labelStatus->setText("Configuration file read OK  [" +  destFilePath.section("/",-1,-1) + "]" );
            ConfigFileOperation_status=ConfigFileOperation_status_idle;
            break;

        case ConfigFileOperation_status_CPU_inProgress:
            timerBoot->stop();
            ui->labelStatus->setText("init CPU update...");
            RestartCPU();

            if (Serial_WaitCharTimeout('k', 5000)!=1) {ConfigFileOperation_errorCode=1; ConfigFileOperation_status=ConfigFileOperation_status_CPU_endKO; timerBoot->start(); break;}
            Serial_PutChar('k');
            Serial_PutChar('M');

            if (Serial_WaitCharTimeout('M', 3000)!=1) {ConfigFileOperation_errorCode=2; ConfigFileOperation_status=ConfigFileOperation_status_CPU_endKO; timerBoot->start(); break;}

            ui->labelStatus->setText("Erasing Flash...");
            if (Serial_WaitCharTimeout('h', 15000)!=1) {ConfigFileOperation_errorCode=3; ConfigFileOperation_status=ConfigFileOperation_status_CPU_endKO; timerBoot->start(); break;}
            ui->labelStatus->setText("Writing Flash...");



            SerialIndex = 0;
            do
            {
                if (myFileArray[myFileArray_index] != 'S')
                {
                    ConfigFileOperation_errorCode=5; ConfigFileOperation_status=ConfigFileOperation_status_CPU_endKO; timerBoot->start();
                    break;
                }
                RecType = myFileArray[myFileArray_index+1];

                NumByte = CPU_ConvertToNum(myFileArray_index+2);
                myFileArray_index+=4;
                if ( (RecType=='0')||(RecType=='3')||(RecType=='8')||(RecType=='9') || (NumByte==4) )
                {
                    myFileArray_index+=(NumByte+1)*2;
                    continue;
                }

                if (WriteByteMasterNext(NumByte, 0) != 0)
                {
                    ConfigFileOperation_errorCode=6; ConfigFileOperation_status=ConfigFileOperation_status_CPU_endKO; timerBoot->start();
                    break;
                }

                for (i = 0; i<(NumByte-1); i++)
                {
                    dato8 = CPU_ConvertToNum(myFileArray_index);
                    myFileArray_index+=2;
                    if (WriteByteMasterNext(dato8, 0) != 0)
                    {
                        ConfigFileOperation_errorCode=7; ConfigFileOperation_status=ConfigFileOperation_status_CPU_endKO; timerBoot->start();
                        break;
                    }
                }
                myFileArray_index+=4;


            } while (RecType < '7');

            WriteByteMasterNext(4, 0);
            WriteByteMasterNext(0, 1);
            dato8 = 0;
            for (i = 0; i < SerialIndex; i++)
                dato8 += TxBufCPU[i];
            if (Serial_WaitCharTimeout(dato8, 90)!=1)
            {
                ConfigFileOperation_errorCode = 8;
                ConfigFileOperation_status = ConfigFileOperation_status_CPU_endKO;
                timerBoot->start();
                break;
            }

            RestartCPU();
            ConfigFileOperation_status=ConfigFileOperation_status_CPU_endOK;
            timerBoot->start();
            break;

        case ConfigFileOperation_status_CPU_endKO:
            timerBoot->stop();
            ui->buttonStart->setVisible(true);
            ui->labelStatus->setText("CPU update failed - " + QString::number(ConfigFileOperation_errorCode));
            ConfigFileOperation_status=ConfigFileOperation_status_idle;
            ComCommandRequest=ComCommandRequest_CheckStatus_req;
            ComStatus=ComStatus_Idle;
            break;

        case ConfigFileOperation_status_CPU_endOK:
            timerBoot->stop();
            foreverDisableBtnStartVMC();
            ui->buttonStart->setVisible(true);
            ui->labelStatus->setText("CPU update OK");
            logToHistoryFile();
            ConfigFileOperation_status=ConfigFileOperation_status_idle;
            ComCommandRequest=ComCommandRequest_CheckStatus_req;
            ComStatus=ComStatus_Idle;
            break;


    case ConfigFileOperation_status_Audit_endKO:
        timerBoot->stop();
        ui->labelStatus->setText("Audit read failed " + QString::number(ConfigFileOperation_errorCode));
        ConfigFileOperation_status=ConfigFileOperation_status_idle;
        break;

    case ConfigFileOperation_status_Audit_endOK:
        timerBoot->stop();
        Audit_saveToFile();
        ui->labelStatus->setText("Audit read OK  [" +  destFilePath.section("/",-1,-1) + "]" );
        ConfigFileOperation_status=ConfigFileOperation_status_idle;
        break;

    }
}




bool isUsbPresent(void)
{
    bool res=false;

    if (QDir(Folder_PenDrive).exists())
    {
        res=true;
    }
    return res;
}


void FormBoot::on_buttonStart_clicked()
{
    if (bBtnStartVMCEnabled == false)
        return;

    FormBoot_ActiveStatus=2;
    this->close();
}



void FormBoot::on_buttonTouchCalib_clicked()
{

}




unsigned char Serial_WaitCharTimeout(unsigned char datoWait, unsigned long timeoutLoopVal)
{
    unsigned char RxData;
    do
    {
        if (serialCPU->bytesAvailable()!=0)
        {
            serialCPU->getChar((char*)&RxData);
            if (RxData==datoWait) return 1;
        }

        RxData=5;
        myDelay(RxData);
        timeoutLoopVal-=RxData;
    } while(timeoutLoopVal>0);
    return 0;
}


void Serial_PutChar(unsigned char datoSend)
{
    QByteArray Qdata(1, 0);
    Qdata[0] = datoSend;
    serialCPU->write(Qdata, 1);
    QCoreApplication::processEvents();
    serialCPU->flush();
    QCoreApplication::processEvents();
}


void RestartCPU(void)
{
    serialCPU->readAll();
    Serial_PutChar('#');
    Serial_PutChar('U');
    Serial_PutChar(4);
    Serial_PutChar('#'+'U'+4);
}

unsigned char CPU_ConvertToNum(int index_p)
{
    unsigned char AA,BB;
    AA=myFileArray[index_p];
    BB=myFileArray[index_p+1];

    if (AA >= '0' && AA <= '9') AA = (unsigned char)(AA - (unsigned char)'0');
    else AA = (unsigned char)(AA - 55);
    if (BB >= '0' && BB <= '9') BB = (unsigned char)(BB - (unsigned char)'0');
    else BB = (unsigned char)(BB - 55);
    return (unsigned char)(BB + AA * 16);
}


unsigned char WriteByteMasterNext(unsigned char dato_8, unsigned char isLastFlag)
{
    unsigned char checksum_counter = 0;
    int i;
    if (isLastFlag == 0)
    {
        TxBufCPU[SerialIndex++] = dato_8;
        if (SerialIndex != max_check_cont) return 0;
    }


    for (i=0; i<SerialIndex;i++){ QarrayTX[i]=TxBufCPU[i];}
    serialCPU->write(QarrayTX, SerialIndex);
    serialCPU->flush();

    for (int i = 0; i < SerialIndex; i++) checksum_counter += TxBufCPU[i];
    myDelay(4);
    if (isLastFlag == 1) return 0;

    if (Serial_WaitCharTimeout(checksum_counter, 90)!=1) { return 1; }
    SerialIndex = 0;
    return 0;
}



static bool copyRecursively(const QString &srcFilePath, const QString &tgtFilePath)
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir()) {
        QDir targetDir(tgtFilePath);
        targetDir.cdUp();
        if (!targetDir.mkdir(QFileInfo(tgtFilePath).fileName()))

            { QMessageBox::information(NULL, "t_A", tgtFilePath); return false; }
        QDir sourceDir(srcFilePath);
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        foreach (const QString &fileName, fileNames) {
            const QString newSrcFilePath
                    = srcFilePath + QLatin1Char('/') + fileName;
            const QString newTgtFilePath
                    = tgtFilePath + QLatin1Char('/') + fileName;
            if (!copyRecursively(newSrcFilePath, newTgtFilePath))

                { QMessageBox::information(NULL, "t_B", newSrcFilePath); return false; }
        }
    } else {
        if (!QFile::copy(srcFilePath, tgtFilePath))

            { QMessageBox::information(NULL, "t_C", srcFilePath); return false; }

    }
    return true;
}


void myDelay(long msec_p)
{
    QTime dieTime= QTime::currentTime().addMSecs(msec_p);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}



void FormBoot::on_buttonWriteSettings_clicked()
{
    int i;
    if (ui->listVMCSettingsFiles->selectedItems().count()==0)
        return;
    QListWidgetItem *item = ui->listVMCSettingsFiles->selectedItems().at(0);
    QString sourceFilePath = Folder_VMCSettingsPenDrive + "/" +  item->text();
    destFilePath = Folder_VMCSettings + "/" +  item->text();

    //prepara i dati per il log della hisotry
    history_type = HISTORY_TYPE_DA3;
    history_lastFileName = item->text();



    QDir dest_dir(Folder_VMCSettings);
    if (dest_dir.exists())
    {
        dest_dir.removeRecursively();
    }
    dest_dir.mkpath(".");

    QFile::copy(sourceFilePath, destFilePath);

    QFile myFile(destFilePath);
    if (!myFile.open(QIODevice::ReadOnly))
    {
        ui->labelStatus->setText("Configuration file write failed - 1a");
        return ;
    }

    ui->buttonStart->setVisible(false);

    for (i=0;i<ConfigFileSize;i++)
        myFileArray[i]=0;
    myFileArray = myFile.readAll();
    myFile.close();

    myFileArray_index=0;
    ConfigFileOperation_status = ConfigFileOperation_status_Write_inProgress;
    ComCommandRequest = ComCommandRequest_WriteConfigFile_req;

    ui->labelStatus->setText("writing config file...");
    timerBoot->start(500);
}




void FormBoot::on_buttonReadSettings_clicked()
{
    int i;
    for (i=0;i<ConfigFileSize;i++)
        myFileArray[i]=0;
    myFileArray_index=0;
    ConfigFileOperation_status=ConfigFileOperation_status_Read_inProgress;
    ComCommandRequest=ComCommandRequest_ReadConfigFile_req;


    ui->labelStatus->setText("reading config file...");
    timerBoot->start(500);
}


int ReadConfigFile_saveToFile(void)
{
    /* dato che non abbiamo modo di memorizzare nella history il nome del file della GPU (perchè è il bootloader che si
     * occupa di aggiornare il file GPU), scrivo nell'history la versione della GPU in modo da avere comunque
     * una informazione utile
     */
    char GPUFilenameForHistory[64];
    sprintf (GPUFilenameForHistory, "GPU v %d.%d.%d", GPU_VERSION_MAJOR, GPU_VERSION_MINOR, GPU_VERSION_BUILD);
    History::addEntry (HISTORY_TYPE_GPU, GPUFilenameForHistory);





    int res=0;
    int numFile;


    numFile=0;
    do
    {
        destFilePath = Folder_VMCSettingsPenDrive + "/Read_" +  QString::number(numFile) + ".da3";
        numFile++;
    } while(QFile(destFilePath).exists());


    QFile myFile(destFilePath);
    myFile.open(QIODevice::WriteOnly);
    myFile.write(myFileArray, ConfigFileSize);

    History::appendAllDataToQFile(&myFile);

    myFile.flush();
    fsync(myFile.handle());
    myFile.close();
    sync();


    return res;
}






void FormBoot::on_buttonWriteCPU_clicked()
{
    if (ui->listCPUFiles->selectedItems().count()==0)
        return;
    QListWidgetItem *item = ui->listCPUFiles->selectedItems().at(0);
    QString sourceFilePath = Folder_FirmwareCPUPenDrive + "/" +  item->text();
    destFilePath = Folder_FirmwareCPU + "/" +  item->text();


    //prepara i dati per il log della hisotry
    history_type = HISTORY_TYPE_CPU;
    history_lastFileName = item->text();



    QDir dest_dir(Folder_FirmwareCPU);
    if (dest_dir.exists())
    {
        dest_dir.removeRecursively();
    }

    dest_dir.mkpath(".");

    QFile::copy(sourceFilePath, destFilePath);

    QFile myFile(destFilePath);
    if (!myFile.open(QIODevice::ReadOnly))
    {
        ui->labelStatus->setText("CPU file write failed - 1a");
        return ;
    }
    myFileArray.resize(myFile.size());
    myFileArray = myFile.readAll();
    myFile.close();

    myFileArray_index=0;
    ConfigFileOperation_status = ConfigFileOperation_status_CPU_inProgress;

    ComStatus = ComStatus_Disabled;

    ui->buttonStart->setVisible(false);
    ui->labelStatus->setText("init CPU update...");
    timerBoot->start(300);
}


/* Copia tutti i file .ttf e otf presenti nella cartella fonts della GUI e li mette in /usr/lib/fonts (solo se tali file
 * non esistono già nel SO).
 * Ritorna il num di file copiati. Generalmente è necessario fare un reboot dopo aver "installato" un font nel SO
 *
 * ATTENZIONE: la versione OS 5 non supporta più i font di tipo OTF
 */
int FormBoot::instalFonts(const QString *srcFolder)
{
    int nFontCopiati = 0;
    QString destFilePath;
    QString destFullFilename;


    QDir dir(*srcFolder);
    if (dir.exists())
    {
        dir.setFilter(QDir::Files);
        QFileInfoList list = dir.entryInfoList();

        foreach (QFileInfo finfo, list)
        {
            if ( QString::compare (finfo.suffix(), "ttf", Qt::CaseInsensitive) == 0 ||
                 QString::compare (finfo.suffix(), "otf", Qt::CaseInsensitive) == 0)
            {
                destFilePath = "/usr/lib/fonts/";
                destFilePath += finfo.fileName();

                if ( !QFile(destFilePath).exists() )
                {
                    //ui->labelStatus->setText(destFilePath); myDelay(500);
                    DEBUG_MSG ("  copying...");
                    QFile::copy(finfo.absoluteFilePath(), destFilePath);
                    nFontCopiati++;
                }
            }
        }
    }


    return nFontCopiati;
}


void FormBoot::on_buttonWriteGUI_clicked()
{
    bool res;
    try
    {
        if (ui->listGUIFiles->selectedItems().count()==0) return;
        QListWidgetItem *item = ui->listGUIFiles->selectedItems().at(0);
        QString sourceFilePath = Folder_GUIPenDrive + "/" +  item->text();
        destFilePath = Folder_GUI;


        ui->labelStatus->setText("erasing current GUI..."); myDelay(20);
        QDir dest_dir(Folder_GUI);
        if (dest_dir.exists())
        {
            dest_dir.removeRecursively();
        }


        ui->buttonStart->setVisible(false);
        ui->labelStatus->setText("writing GUI..."); myDelay(20);
        res = copyRecursively(sourceFilePath, destFilePath);

        sourceFilePath = Folder_GUI + "/web/fonts";
        int nFontInstallati = instalFonts(&sourceFilePath);

        sync();

        if (res==false)
            ui->labelStatus->setText("GUI write failed");
        else
        {
            History::addEntry (HISTORY_TYPE_GUI, item->text().toStdString().c_str());

            ui->labelStatus->setText("GUI write OK");
            if (nFontInstallati)
            {
                ui->labelWarning->setText("One or more fonts have been installed. You should reboot the machine");
                ui->labelWarning->setVisible(true);
            }
        }

    }
    catch (...)
    {
        ui->labelStatus->setText("GUI write failed -1");
    }

    ui->buttonStart->setVisible(true);
}


void FormBoot::on_buttonReadGUI_clicked()
{
    bool res;
    try
    {
        QString sourceFilePath = Folder_GUI;

        int numFile=0;
        do
        {
            destFilePath = Folder_GUIPenDrive + "/GUI_Read_" +  QString::number(numFile);
            numFile++;
        } while(QDir(destFilePath).exists());


        ui->labelStatus->setText("reading GUI..."); myDelay(20);
        res = copyRecursively(sourceFilePath, destFilePath);

        sync();

        if (res==true) ui->labelStatus->setText("GUI read OK");
        else ui->labelStatus->setText("GUI read failed");
    }
    catch (...)
    {
        ui->labelStatus->setText("GUI read failed -1");
    }
}





void FormBoot::on_buttonWriteManual_clicked()
{
    bool res;
    try
    {
        if (ui->listManualFiles->selectedItems().count()==0) return;
        QListWidgetItem *item = ui->listManualFiles->selectedItems().at(0);
        QString sourceFilePath = Folder_ManualPenDrive + "/" +  item->text();

        destFilePath = Folder_Manual + "/" +  item->text();


        QDir root_dir(Folder_Manual);
        /*
        if (!root_dir.exists())
        {
            root_dir.mkpath(".");
        }
        */
        if (root_dir.exists())
        {
            root_dir.removeRecursively();
        }
        root_dir.mkpath(".");



        ui->labelStatus->setText("erasing current Manual..."); myDelay(20);
        QDir dest_dir(destFilePath);
        if (dest_dir.exists())
        {
            dest_dir.removeRecursively();
        }


        ui->labelStatus->setText("copying Manual..."); myDelay(20);
        res = copyRecursively(sourceFilePath, destFilePath);

        if (res==true) ui->labelStatus->setText("Manual copy OK");
        else ui->labelStatus->setText("Manual copy failed");
    }
    catch (...)
    {
        ui->labelStatus->setText("Manual copy failed -1");
    }
}




void FormBoot::on_buttonReadAudit_clicked()
{
    int i;
    for (i=0;i<AUDIT_MAX_FILESIZE;i++) myAuditArray[i]=0;
    myFileArray_index=0;
    ConfigFileOperation_status=ConfigFileOperation_status_Audit_inProgress;
    ComCommandRequest=ComCommandRequest_ReadAudit_req;

    ui->labelStatus->setText("reading audit (machine=>USB) ...");
    timerBoot->start(500);
}


int Audit_saveToFile(void)
{
    int res=0;
    int numFile;
    int nWritten=0;


    numFile=0;
    do
    {
        destFilePath = Folder_AuditPenDrive + "/Eva_A_" +  QString::number(numFile) + ".txt";
        numFile++;
    } while(QFile(destFilePath).exists());



    QDir dest_dir(Folder_AuditPenDrive);
    if (!dest_dir.exists())
    {
        dest_dir.mkpath(".");
        sync();
    }


    QFile myFile(destFilePath);
    myFile.open(QIODevice::WriteOnly);
    nWritten=myFile.write(myAuditArray, myFileArray_index);
    myFile.flush();
    fsync(myFile.handle());
    myFile.close();
    sync();

    if(nWritten<0) res=nWritten;
    return res;
}


/**********************************************************************
 * copia la cartella lang su una chiavetta USB in rhea/lang
 *
 */
void FormBoot::on_btnReadLang_clicked()
{
    priv_langCopy (Folder_languages, Folder_LangPenDrive);
}

/**********************************************************************
 *
 * copia tutti i file *.lng dalla USB all'HD locale
 *
 */
void FormBoot::on_btnWriteLang_clicked()
{
    priv_langCopy (Folder_LangPenDrive, Folder_languages);
}

//**********************************************************************
bool FormBoot::priv_langCopy (const QString &srcFolder, const QString &dstFolder) const
{
    ui->btnReadLang->setEnabled(false);
    ui->btnWriteLang->setEnabled(false);

    ui->labelStatus->setText("copying files...");
    myDelay(20);

    //se la directory dst non esiste, la creo, se esiste, la svuoto
    {
        QDir root_dir(dstFolder);
        if (root_dir.exists())
            root_dir.removeRecursively();
        root_dir.mkpath(".");
    }

    //copia
    QDir directory(srcFolder, "*.lng", QDir::Unsorted, QDir::Files);
    QStringList list = directory.entryList();
    bool bFailed = false;
    for (int i=0; i<list.count(); i++)
    {
        QString src = srcFolder +"/" +list.at(i);
        QString dst = dstFolder +"/" +list.at(i);

        ui->labelStatus->setText(list.at(i));
        try
        {
            if (QFile::exists (dst))
                QFile::remove(dst);
            if (!QFile::copy(src, dst))
            {
                ui->labelStatus->setText(QString("Error copying ") +src);
                bFailed = true;
                i = list.count()+1;
            }
        }
        catch (const std::exception& e)
        {
            bFailed = true;
            ui->labelStatus->setText(QString("Error: ") +e.what());
            i = list.count()+1;
        }
    }

    if (!bFailed)
        ui->labelStatus->setText("Done!");

    ui->btnReadLang->setEnabled(true);
    ui->btnWriteLang->setEnabled(true);

    return (!bFailed);
}
