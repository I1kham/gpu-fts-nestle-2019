#include "header.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "formprog.h"
#include "formboot.h"
#include "history.h"
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <qvideowidget.h>
#include <qfileinfo.h>
#include <QSerialPort>
#include <QTimer>
#include <QDateTime>
#include <QThread>
#include <QFontDatabase>
#include <QWebFrame>
#include "formdebug.h"
#include "Utils.h"
#include "../rheaGUIBridge/GUIBridge.h"

void    VMCcom(sLanguage *language, MainWindow *mainWindow);

QSerialPort* serialCPU;
QTimer *timer;
QTimer *timer_TX;



/* GIX 2018-12-17
 * La nuova CPU invia un ulteriore byte durante il comando "C". Questo btye serve per identificare la "protocol version".
 * In base alla "protocol version", ora ed in futuro potremo discriminare tra le varianti del protocollo ed adattare il codice
 * di conseguenza*/
unsigned char protocol_version=0;

/* GIX 2018-12-17
 * Questo parametro esiste solo per coerenza con il codice GPU della TT PCAP. In questa particolare GPU infatt il beepSelezione non
 * è utilizzato. In ogni caso la CPU lo invia costantemente nel messaggio di stato "U"/"B" e no lo leggiamo
 */
unsigned int  beepSelezioneLenMSec=0;


/* ritornata dalla CPU durante la risposta alla richiesta di stato, dovrebbe indicare la presenza o meno
 * del bicchiere. Pare che valga 1 se il bicchiere è assente, 0 se il bicchiere è presente
 */
unsigned char CupAbsentStatus_flag=0;



/*  Se == 1, NON mostra il btn di STOP durante l'emissione della sezione
 *  Questo parametro viene settato nella risposta di stato della cpu
 */
unsigned char bShowDialogStopSelezione = 0;

/*
 * cerca "GIX 2018 05 04" per il commento su come funziona questa variabile
 */
enum eStatoPreparazioneBevanda
{
    eStatoPreparazioneBevanda_unsupported = 0,
    eStatoPreparazioneBevanda_doing_nothing = 0x01,
    eStatoPreparazioneBevanda_wait = 0x02,
    eStatoPreparazioneBevanda_running = 0x03
};
eStatoPreparazioneBevanda statoPreparazioneBevanda = eStatoPreparazioneBevanda_unsupported;


unsigned char VMCstate = 0;
unsigned char VMCstate_old=0;
unsigned char VMCerrorCode;
unsigned char VMCerrorType;

unsigned char RxBufCPU[MaxLenBufferComCPU_Rx];
unsigned char TxBufCPU[MaxLenBufferComCPU_Tx];
QByteArray RxArrayCPU("a", MaxLenBufferComCPU_Rx+1);
QByteArray TxArrayCPU("a", MaxLenBufferComCPU_Tx+1);
unsigned char RxCurrentCPU=0;
unsigned char TxCurrentCPU=0;
unsigned char RxSizeCPU=0;
unsigned char TxSizeCPU=0;
unsigned char ComStatus=0;
unsigned char ComCommandRequest=0;
unsigned int Com_NumTimeoutChar;
unsigned int Com_NumTotTimeoutCommand=0;



QChar MsgLcdCPU[MAXLEN_MSG_LCD+2];
unsigned char MsgLcdCPUImportanceLevel = 0xff;
char userCurrentCredit[16] = {0};


unsigned char flag_initial_param_received=0;

QString CPU_version;
unsigned int Prices[NUM_MAX_SELECTIONS];

unsigned char FormBoot_ActiveStatus=0;


QString Folder_root;
QString Folder_VMCSettings;
QString Folder_FirmwareCPU;
QString Folder_GUI;
QString Folder_Manual;
QString Folder_languages;




QByteArray myFileArray(ConfigFileSize, 0);
QByteArray myAuditArray(AUDIT_MAX_FILESIZE, 0);
int myFileArray_index;
unsigned char ConfigFileOperation_status;
unsigned char ConfigFileOperation_errorCode;


SelectionAvailability   selAvailability;

//********************************************************************************
MainWindow::MainWindow(const HThreadMsgR hQMessageFromWebserverIN, const HThreadMsgW hQMessageToWebserverIN) :
        QMainWindow(NULL),
        ui(new Ui::MainWindow),
        lastReceivedCPUMsg(MAXLEN_MSG_LCD+2),
        currentShownCPUMsg(MAXLEN_MSG_LCD+2)
{
    isInterruptActive=false;
    timeToSendCPUMsgToGUI_MSec = 0;
    hQMessageFromWebserver = hQMessageFromWebserverIN;
    hQMessageToWebserver = hQMessageToWebserverIN;

#ifdef SHOW_DEBUG_WINDOW_WITH_COM_MESSAGES_AT_STARTUP
    enableFormDEBUG();
#endif

    currentShownCPUMsg = "";
    lastReceivedCPUMsg = "";
    lang_init (&language);
    lang_open( &language, "GB");
    
    debug_ct_pageStandBy=0;
    debug_lastTimePageStandByWasShown=0;

    timeFormStatusChangedMSec=0;
    bBevandaInPreparazione=0;
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    hideMouse();

    selAvailability.reset();
    for (int i=0; i<NUM_MAX_SELECTIONS; i++)
        Prices[i]=0;


    dialogAButton = new FormResetGrnCounter(this);
    dialogAButton->hide();


    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::AutoLoadImages, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);



    Folder_root = qApp->applicationDirPath();
    Folder_VMCSettings = Folder_root + "/rheaData";
    Folder_FirmwareCPU = Folder_root + "/rheaFirmwareCPU01";
    Folder_GUI = Folder_root + "/rheaGUI";
    Folder_Manual = Folder_root + "/rheaManual";
    Folder_languages = Folder_root + "/lang";





    ui->webView->setGeometry(0,0,ScreenW,ScreenH);
    ui->webView->setMouseTracking(false);
    ui->webView->raise();
    ui->webView->settings()->setAttribute(QWebSettings::PluginsEnabled,true);
    ui->webView->settings()->setAttribute(QWebSettings::AutoLoadImages, true);
    ui->webView->settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);
    ui->webView->setEnabled(false);



    serialCPU = new QSerialPort(this);
    serialCPU->setPortName(serialCPU_NAME);
    serialCPU->setBaudRate(QSerialPort::Baud115200);
    serialCPU->setDataBits(QSerialPort::Data8);
    serialCPU->setParity(QSerialPort::NoParity);
    serialCPU->setStopBits(QSerialPort::OneStop);
    serialCPU->setFlowControl(QSerialPort::NoFlowControl);
    if (serialCPU->open(QIODevice::ReadWrite))
    {
        ;
    }
    else
    {
        QMessageBox::information(NULL, tr("Error"), serialCPU->errorString());
    }

    #ifdef DEBUG_KEEP_QUIT_BUTTON
        ui->pushButtonQuit->raise();
    #endif


    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    ui->webView->setEnabled(true);
    ui->webView->setFocus();



    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerInterrupt()));
    timer->start(TIMER_INTERVAL_MSEC);



    setFormStatus(FormStatus_NORMAL, "costruttore");
    if (isUsbPresent() == true)
        setFormStatus(FormStatus_BOOT, "costruttore");
    else
        priv_loadGUIFirstPage();

    ui->webView->setFocus();

}


//********************************************************************************
MainWindow::~MainWindow()
{
    serialCPU->close();
    delete ui;
}



//*****************************************************
void MainWindow::setFormStatus (int i, const char *who)
{
    if (i == formStatus)
        return;
    timeFormStatusChangedMSec = getTimeNowMSec();
    DEBUG_MSG ("MainWindow::setFormStatus(%d), who:%s", i, who);
    formStatus = i;

    if (formStatus == FormStatus_SELECTION_RUNNING)
        bBevandaInPreparazione = 0;
}

//******************************************************
void MainWindow::priv_loadGUIFirstPage()
{
    QString url = "file://" + Folder_GUI + "/web/pageStandby01.html";
    ui->webView->load(QUrl(url));
    DEBUG_MSG ("MainWindow::SetForm_01() URL=%s", url.toStdString().c_str());

    debug_showTreeViewOfInstalledFont();
}

//******************************************************
void MainWindow::priv_showDiaglogResetGroundCounter()
{
    if (!dialogAButton->isVisible())
    {
        dialogAButton->setButtonText(BTN_RESET_GROUND_COUNTER_TEXT);
        dialogAButton->btnWasPressed = 0;
        dialogAButton->show();
    }
}

//******************************************************
void MainWindow::priv_hideDiaglogResetGroundCounter()
{
    if (dialogAButton->isVisible())
        dialogAButton->hide();
}

//******************************************************
void MainWindow::priv_showDiaglogCleanMilker()
{
    if (!dialogAButton->isVisible())
    {
        dialogAButton->setButtonText(BTN_LAVAGGIO_MILKER_TEXT);
        dialogAButton->btnWasPressed = 0;
        dialogAButton->show();
    }
}

//******************************************************
void MainWindow::priv_hideDiaglogCleanMilker()
{
    if (dialogAButton->isVisible())
        dialogAButton->hide();
}

/*****************************************************
 * Prende il msg di stato ricevuto dalla CPU e lo mette i [currentShownCPUMsg] facendone la traduzione se necessario
 */
void MainWindow::priv_translateCPUMessage (const QChar *cpuMsgBuffer, int sizeofCpuMsgBuffer,  unsigned char msgImportanceLevel)
{
    int i;

    //se il messaggio di CPU è lo stesso del giro precedente, non faccio niente in quanto
    //dentro [currentShownCPUMsg] c'è già il messaggio da viualizzare
    i = 0;
    while (i < sizeofCpuMsgBuffer)
    {
        if (cpuMsgBuffer[i]==0x00)
            break;

        if (cpuMsgBuffer[i] != lastReceivedCPUMsg[i])
        {
            i = -1;
            break;
        }
        ++i;
    }

    if (i == -1)
    {
        //a questo punto, sappiamo di aver ricevuto un msg != da quello attualmente visualizzato
        timeToSendCPUMsgToGUI_MSec = 0;

        i = 0;
        while (cpuMsgBuffer[i] != 0x00)
        {
            lastReceivedCPUMsg[i] = cpuMsgBuffer[i];
            ++i;
        }
        lastReceivedCPUMsg[i] = 0x00;


        //se il primo ch diverso da "spazio" è "@", allora stiamo parlano di un messaggio custom
        i = 0;
        while (lastReceivedCPUMsg[i] != 0x00)
        {
            if (lastReceivedCPUMsg[i] != ' ')
                break;
            ++i;
        }


        if (lastReceivedCPUMsg[i] == '@')
        {
            QChar msgTranslated[MAXLEN_MSG_LCD+2];

            int t=0;
            while (lastReceivedCPUMsg[i] != 0x00)
                msgTranslated[t++] = lastReceivedCPUMsg[i++];
            msgTranslated[t] = 0x00;

            //rtrim (msgTranslated);
            if (t > 0)
            {
                --t;
                while (msgTranslated[t] == ' ')
                    msgTranslated[t--] = 0x00;

                lang_translate (&language, msgTranslated, MAXLEN_MSG_LCD+1);

                currentShownCPUMsg = QString (msgTranslated, -1);
            }
            else
                currentShownCPUMsg = "";

        }
        else
        {
            currentShownCPUMsg = lastReceivedCPUMsg;
        }

        currentShownCPUMsg = currentShownCPUMsg.trimmed();
        //currentShownCPUMsg = "[" +QString::number(msgImportanceLevel) + QString("] ") + currentShownCPUMsg;

#ifdef DEBUG_MONITOR_CPU_USAGE
        {
            char sDebug[32];
            sprintf (sDebug,"VMst:%d COMst:%d CPU: %.1f%% ", VMCstate, ComStatus, updateCPUStats());
            currentShownCPUMsg = sDebug + currentShownCPUMsg;
        }
#endif
        DEBUG_MSG("Currentshown:%s!!!", currentShownCPUMsg.toStdString().c_str());
    }


    u64 timeNowMSec = OS_getTimeNowMSec();
    if (timeNowMSec >= timeToSendCPUMsgToGUI_MSec)
    {
        timeToSendCPUMsgToGUI_MSec = timeNowMSec +2000;
        guibridgeEvent_sendCPUMessage (currentShownCPUMsg, msgImportanceLevel);
    }
}

//*****************************************************
void MainWindow::priv_guibridgeAnswer_selectionAvailabilityList (u16 handlerID, int numSel, const SelectionAvailability &selAvailList) const
{
    //2 byte per handlerID
    //1 byte per indicare il num di selezioni
    //1 bit per ogni selezione
    const u8 SIZE = 4 + (numSel/8);
    u8 data[SIZE];

    memset (data, 0, SIZE);
    data[0] = (u8)((handlerID & 0xFF00) >> 8);
    data[1] = (u8)(handlerID & 0x00FF);
    data[2] = (u8)numSel;

    u8 byte = 3;
    u8 bit = 0x01;
    for (u8 i=0; i<numSel; i++)
    {
        if (selAvailList.isAvail(i+1))
            data[byte] |= bit;

        if (bit == 0x80)
        {
            bit = 0x01;
            byte++;
        }
        else
            bit <<= 1;
    }

    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_GPU_EVENT, data, SIZE);
}

//*****************************************************
void MainWindow::priv_guibridgeAnswer_selectionPricesList (u16 handlerID, int numSel, const unsigned int *priceListIN) const
{
    //2 byte per handlerID
    //1 byte per indicare il num di selezioni
    //2 byte per la lunghezza della stringa (comprensiva di 0x00 finale)
    //n byte stringa contenenti la lista dei prezzi formattati, separati da §

    const u16 MAX_STR_SIZE = 512;

    char priceList[MAX_STR_SIZE];
    memset (priceList, 0, sizeof(priceList));
    for (int i=0; i<numSel; i++)
    {
        char s[32];
        utils::formatCurrency (priceListIN[i], 2, '.', s, sizeof(s));

        if (i > 0)
            strcat (priceList, "§");
        strcat (priceList, s);
    }

    const u16 priceListLen = 1 + strlen(priceList);

    u8 data[MAX_STR_SIZE];
    u16 ct = 0;
    data[ct++] = (u8)((handlerID & 0xFF00) >> 8);
    data[ct++] = (u8)(handlerID & 0x00FF);
    data[ct++] = (u8)numSel;
    data[ct++] = (u8)((priceListLen & 0xFF00) >> 8);
    data[ct++] = (u8)(priceListLen & 0x00FF);
    memcpy (&data[ct], priceList, priceListLen);
    ct += priceListLen;

    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_GPU_EVENT, data, ct);
}

//*****************************************************
void MainWindow::priv_guibridgeAnswer_credit (u16 handlerID, const char *credit) const
{
    //2 byte per handlerID
    //8 byte stringa per indicare il credito

    u8 data[16];
    memset (data, 0, sizeof(data));

    data[0] = (u8)((handlerID & 0xFF00) >> 8);
    data[1] = (u8)(handlerID & 0x00FF);

    u32 n = (u32)strlen(credit);
    if (n < 8)
        memcpy (&data[2], credit, n);
    else
        memcpy (&data[2], credit, 8);

    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_GPU_EVENT, data, 10);
}

/*****************************************************
 * inviano l'evento alla GUI
 *
 */
void MainWindow::guibridgeEvent_selectionAvailabilityUpdated (int numSel, const SelectionAvailability &selAvailList) const
{
    priv_guibridgeAnswer_selectionAvailabilityList (guibridge::eEventType_selectionAvailabilityUpdated, numSel, selAvailList);
}
void MainWindow::guibridgeEvent_selectionPricesUpdated (int numSel, const unsigned int *priceList) const
{
    priv_guibridgeAnswer_selectionPricesList (guibridge::eEventType_selectionPricesUpdated, numSel, priceList);
}
void MainWindow::guibridgeEvent_selectionReqStatus (eSelectionReqStatus status) const
{
    const u16 handlerID = guibridge::eEventType_selectionRequestStatus;

    u8 data[4];
    u8 ct = 0;
    data[ct++] = (u8)((handlerID & 0xFF00) >> 8);
    data[ct++] = (u8)(handlerID & 0x00FF);
    data[ct++] = (u8)status;

    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_GPU_EVENT, data, ct);
}
void MainWindow::guibridgeEvent_sendCPUMessage (const QString &msg, u8 importanceLevel) const
{
    const u16 handlerID = guibridge::eEventType_cpuMessage;

    QByteArray inUtf8 = msg.toUtf8();
    const char *msgBytes = inUtf8.constData();
    u16 msgLenInByte = inUtf8.length();

    u8 data[512];
    u8 ct = 0;
    data[ct++] = (u8)((handlerID & 0xFF00) >> 8);
    data[ct++] = (u8)(handlerID & 0x00FF);
    data[ct++] = (u8)importanceLevel;
    data[ct++] = (u8)((msgLenInByte & 0xFF00) >> 8);
    data[ct++] = (u8)(msgLenInByte & 0x00FF);

    for (int i=0; i<msgLenInByte; i++)
        data[ct++] = msgBytes[i];

    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_GPU_EVENT, data, ct);
}


/*****************************************************
 * Controlla la msgQ del server di GUI ed eventualmente
 * processa i messaggi in ingresso
 */
void MainWindow::priv_handleGUIBridgeServerMessages()
{
    rhea::thread::sMsg msg;
    if (!rhea::thread::popMsg(hQMessageFromWebserver, &msg))
        return;


    if ((msg.what & 0xFF00) == 0x0100)
    {
        //è un msg da parte del server che prevede un handlerID da utilizzare per la risposta
        u16 handlerID = (u16)(msg.paramU32 & 0x0000FFFF);

        switch (msg.what)
        {
        default:
            //comando non suportato...
            break;

        case GUIBRIDGE_REQ_SELAVAILABILITY:
            priv_guibridgeAnswer_selectionAvailabilityList(handlerID, NUM_MAX_SELECTIONS, selAvailability);
            break;

        case GUIBRIDGE_REQ_SELPRICES:
            priv_guibridgeAnswer_selectionPricesList(handlerID, NUM_MAX_SELECTIONS, Prices);
            break;

        case GUIBRIDGE_REQ_STARTSELECTION:
            {
                unsigned char selNum = (unsigned char)((msg.paramU32 & 0xFFFF0000) >> 16);

                if (statoPreparazioneBevanda == eStatoPreparazioneBevanda_unsupported)
                {
                    //Segnalo l'errore alla GUI
                    guibridgeEvent_selectionReqStatus(eSelectionReqStatus_aborted);
                }
                else if (statoPreparazioneBevanda != eStatoPreparazioneBevanda_doing_nothing)
                {
                    //Segnalo l'errore alla GUI
                    guibridgeEvent_selectionReqStatus(eSelectionReqStatus_aborted);
                }
                else
                {
                    //chiedo alla CPU di iniziare la selezione
                    setButtonKeyNum (selNum);
                    setFormStatus(FormStatus_SELECTION_RUNNING, "GUIBRIDGE_REQ_STARTSELECTION");
                    guibridgeEvent_selectionReqStatus(eSelectionReqStatus_waitingCPU);
                }

            }
            break;

        case GUIBRIDGE_REQ_STOPSELECTION:
            //ho ricevuto una richeista di stop della selezione corrente
            DEBUG_MSG ("Selezione stoppata da utente (01)");
            setButtonKeyNum (0xFF);
            break;

        case GUIBRIDGE_REQ_CREDIT:
            priv_guibridgeAnswer_credit(handlerID, userCurrentCredit);
            break;

        } //switch (msg.what)
    }

    rhea::thread::deleteMsg(msg);
}

//*****************************************************
void MainWindow::timerInterrupt()
{
    if (isInterruptActive)
        return;
    isInterruptActive=true;

    //vediamo se c'è un messaggio dal server di guibridge
    priv_handleGUIBridgeServerMessages();


    //Aggiornamento comunicazione seriale
    VMCcom (&language, this);


    switch (getFormStatus())
    {
    case FormStatus_BOOT:
        if(FormBoot_ActiveStatus == 0)
        {
        //    QApplication::setOverrideCursor(Qt::PointingHandCursor);
            FormBoot_ActiveStatus++;
            myFormBoot.show();

        }

        if(FormBoot_ActiveStatus == 2)
        {
            FormBoot_ActiveStatus=0;
            priv_loadGUIFirstPage();
            setFormStatus(FormStatus_NORMAL, "timerInterrupt(5)");
        }
        break;

    case FormStatus_PROG:
        priv_translateCPUMessage (MsgLcdCPU, MAXLEN_MSG_LCD+1, MsgLcdCPUImportanceLevel);
        myFormProg.updateLabelStatusProg(currentShownCPUMsg);

        if(VMCstate != VMCSTATE_PROGRAMMAZIONE)
        {
            setFormStatus(FormStatus_NORMAL, "timerInterrupt(6)");
            setButtonKeyNum(0);
            myFormProg.close();
        }
        break;

    case FormStatus_NORMAL:
        priv_handleFormStatus_NORMAL();
        break;

    case FormStatus_SELECTION_RUNNING:
        priv_handleFormStatus_SELECTION_RUNNING();
        break;
    }

    isInterruptActive=false;
}

//********************************************************
void MainWindow::priv_handleFormStatus_NORMAL()
{
    priv_translateCPUMessage (MsgLcdCPU, MAXLEN_MSG_LCD+1, MsgLcdCPUImportanceLevel);

    if (VMCSTATE_SHOW_DIALOG_RESET_GRND_COUNTER == VMCstate && dialogAButton->isVisible())
    {
        if (dialogAButton->btnWasPressed)
        {
            priv_hideDiaglogResetGroundCounter();
            setButtonKeyNum(1);
        }
    }

    if (VMCSTATE_SHOW_DIALOG_LAVAGGIO_MILKER == VMCstate && dialogAButton->isVisible())
    {
        if (dialogAButton->btnWasPressed)
        {
            priv_hideDiaglogCleanMilker();
            setButtonKeyNum(10);
        }
    }


    if(VMCstate != VMCstate_old)
    {
        DEBUG_MSG("VMCState=%d", VMCstate);

        if (dialogAButton->isVisible())
        {
            if (VMCstate != VMCSTATE_SHOW_DIALOG_RESET_GRND_COUNTER && VMCstate != VMCSTATE_SHOW_DIALOG_LAVAGGIO_MILKER)
                dialogAButton->hide();
        }

        switch(VMCstate)
        {
        case VMCSTATE_DISPONIBILE:
            ui->webView->setEnabled(true);
            hideMouse();
            break;

        case VMCSTATE_INITIAL_CHECK:
            hideMouse();
            break;

        case VMCSTATE_COM_ERROR:
        case VMCSTATE_ERROR:
            selAvailability.reset();
            this->guibridgeEvent_selectionAvailabilityUpdated(NUM_MAX_SELECTIONS, selAvailability);
            break;

        case VMCSTATE_SHOW_DIALOG_RESET_GRND_COUNTER:
            if (!dialogAButton->isVisible())
                priv_showDiaglogResetGroundCounter();
            break;

        case VMCSTATE_SHOW_DIALOG_LAVAGGIO_MILKER:
            if (!dialogAButton->isVisible())
                priv_showDiaglogCleanMilker();
            break;

        case VMCSTATE_PROGRAMMAZIONE:
         //   QApplication::setOverrideCursor(Qt::PointingHandCursor);
            setFormStatus (FormStatus_PROG, "timerInterrupt(1)");
            myFormProg.initForm();
            myFormProg.show();
            break;

        default:
            break;
        }
    }
    VMCstate_old = VMCstate;
}

//********************************************************
void MainWindow::priv_handleFormStatus_SELECTION_RUNNING()
{
    priv_translateCPUMessage (MsgLcdCPU, MAXLEN_MSG_LCD+1, MsgLcdCPUImportanceLevel);

    /* Teoricamente l'intero processeto è ora pilotato da "statoPreparazioneBevanda"
     * Voglio però essere sicuro di aver letto almeno una volta uno stato != da eStatoPreparazioneBevanda_doing_nothing
     * prima di imbarcarmi nel processo.
     * Appena vedo uno stato != da eStatoPreparazioneBevanda_doing_nothing setto bBevandaInPreparazione e parto.
     * Se non vedo questa condizione entro 4/5 second, vuol dire che c'è qualcosa che non va e abortisco*/
    if (bBevandaInPreparazione==0)
    {
        if (statoPreparazioneBevanda != eStatoPreparazioneBevanda_doing_nothing)
        {
            bBevandaInPreparazione=1;
            setButtonKeyNum(0);
            DEBUG_MSG ("waiting for CPU to finish....");
        }
        else if (getTimeNowMSec() - timeFormStatusChangedMSec >= 12000)
        {
            DEBUG_MSG ("priv_handleFormStatus4() -> non ho visto cambiare lo stato da eStatoPreparazioneBevanda_doing_nothing in qualcos'altro");
            priv_onSelezioneTerminataKO();
            return;
        }

        return;
    }


    switch (statoPreparazioneBevanda)
    {
    default:
        DEBUG_MSG ("lo statoPreparazioneBevanda era invalido, val=%d", (int)statoPreparazioneBevanda);
        priv_onSelezioneTerminataKO();
        break;

    case eStatoPreparazioneBevanda_wait:
        //sto aspettando che la CPU decida il da farsi
        if (VMCstate != VMCSTATE_DISPONIBILE && VMCstate != VMCSTATE_PREPARAZIONE_BEVANDA)
        {
            DEBUG_MSG ("bevanda terminata, ero in WAIT ma CPU è andata in uno stato != da DISP o PREP");
            priv_onSelezioneTerminataKO();
        }
        else
            guibridgeEvent_selectionReqStatus(eSelectionReqStatus_waitingCPU); //wait for credit
        break;

    case eStatoPreparazioneBevanda_running:
        //la cpu ha dato l'OK, sta preparando la bevanda
        if (bBevandaInPreparazione==1)
        {
            //è la prima volta che passo di qui
            bBevandaInPreparazione=2;
            timeFormStatusChangedMSec = getTimeNowMSec();

            //visualizza il btn "stop" per interrompere l'attuale selezione
            if (bShowDialogStopSelezione)
                guibridgeEvent_selectionReqStatus(eSelectionReqStatus_preparing_canUseStopBtn);
            else
                guibridgeEvent_selectionReqStatus(eSelectionReqStatus_preparing);
        }


        /*
         * Quanto qui sotto sarebbe formalmente corretto, ma l'ho tolto perchè in questo caso l'intera operazione
         * deve essere pilotata da "StatoPreparazioneBevanda" ignorando gli stati della CPU (che cmq nel 99% dei
         * casi sono corretti e seguono il normale ciclo DISP -> PREP -> DISP
         * Il problema si presente con il btn STOP a seguito del quale la CPU va in PREP ma mette "StatoPreparazioneBevanda" a 1 il
         * che rilsulta in uno stato un po' disordante perchè non puoi essere in PREP ma allo stesso tempo essere in eStatoPreparazioneBevanda_doing_nothing
         *
        if (VMCstate != VMCSTATE_DISPONIBILE && VMCstate != VMCSTATE_PREPARAZIONE_BEVANDA)
        {
            DEBUG_MSG ("bevanda terminata, ero in RUNNING ma CPU è andata in uno stato != da DISP o PREP");
            priv_onSelezioneTerminataKO();
        }
        else
        */
        if (getTimeNowMSec() - timeFormStatusChangedMSec >= TIMEOUT_SELEZIONE_2_MSEC)
        {
            DEBUG_MSG ("TIMEOUT_SELEZIONE_2_MSEC");
            priv_onSelezioneTerminataKO();
        }

        break;

    case eStatoPreparazioneBevanda_doing_nothing:
        if (bBevandaInPreparazione == 2)
            priv_onSelezioneTerminataOK();
        else
            priv_onSelezioneTerminataKO();
        break;

    }
}

//********************************************************
void MainWindow::priv_onSelezioneTerminataOK()
{
    DEBUG_MSG ("Selezione terminata OK");
    History::incCounterSelezioni();
    setButtonKeyNum(0);
    bBevandaInPreparazione=0;

    guibridgeEvent_selectionReqStatus(eSelectionReqStatus_finished); //selection finished
    setFormStatus(FormStatus_NORMAL, "");
}

//********************************************************
void MainWindow::priv_onSelezioneTerminataKO()
{
    DEBUG_MSG ("Selezione terminata KO");
    setButtonKeyNum(0);
    bBevandaInPreparazione=0;

    guibridgeEvent_selectionReqStatus(eSelectionReqStatus_aborted);
    setFormStatus(FormStatus_NORMAL, "");
}










QByteArray QarrayTX(MaxLenBufferComCPU_Tx, 0);
void VMCcom_TX(void)
{
    if(ComStatus==ComStatus_Tx1)
    {
        QByteArray Qdata(1, 0);
        Qdata[0] = TxBufCPU[TxCurrentCPU];
        serialCPU->write(Qdata, 1);

        QCoreApplication::processEvents();
        serialCPU->waitForBytesWritten(2);

        TxCurrentCPU++;
        if (TxCurrentCPU==TxSizeCPU)
        {
            timer_TX->stop();
            ComStatus=ComStatus_Rx;
            Com_NumTimeoutChar=0;
            RxCurrentCPU=0;
            return;
        }


        if(TxCurrentCPU>3)
        {
            timer_TX->setInterval(3);
        }

    }
}

void VMCcom (sLanguage *language, MainWindow *mainWindow)
{
    unsigned char i, k, z;
    unsigned int num_byte_received;

    switch (ComStatus)
    {
        case ComStatus_Idle:
                        switch (ComCommandRequest)
                        {
                            default:
                            case ComCommandRequest_CheckStatus_req:
                                {
                                    unsigned char btnKeyNum = getButtonKeyNum();

                                    //il btn 0xFF è il caso speciale in cui il bottone grafico STOP è stato
                                    //premuto per interrompere la selezione corrente
                                    if (btnKeyNum == 0xFF)
                                    {
                                        btnKeyNum=1;
                                        setButtonKeyNum(0);
                                    }

                                    TxBufCPU[0] = charHeaderPacket;
                                    TxBufCPU[1] = CommandCPUCheckStatus;
                                    TxBufCPU[2] = 9;
                                    TxBufCPU[3] = btnKeyNum;
                                    TxBufCPU[4] = 0;
                                    TxBufCPU[5] = 0;
                                    TxBufCPU[6] = 0;
                                    TxBufCPU[7] = lang_getErrorCode(language);
                                    TxBufCPU[8] = utils::evalChecksum(TxBufCPU, 8);
                                    TxSizeCPU=TxBufCPU[2];
                                    RxSizeCPU=44;
                                    TxCurrentCPU=0;

                                    //lo so che sembra un errore.... una volta dovevo mandare il numero della selezione
                                    //per un tot di volte prima che la CPU lo accettasse, ora devo mandarlo una sola volta
                                    //altrimenti il secondo giro lo interpreta come stop
                                    if(VMCstate != VMCSTATE_PROGRAMMAZIONE && VMCstate != VMCSTATE_SHOW_DIALOG_LAVAGGIO_MILKER)
                                        setButtonKeyNum(0);
                                }
                                break;




                            case ComCommandRequest_InitialParam_req:
                                    TxBufCPU[0] = charHeaderPacket;
                                    TxBufCPU[1] = CommandCPUInitialParam_C;
                                    TxBufCPU[2] = 0;    //spare, non usato

                                    TxBufCPU[3] = GPU_VERSION_MAJOR;
                                    TxBufCPU[4] = GPU_VERSION_MINOR;
                                    TxBufCPU[5] = GPU_VERSION_BUILD;

                                    TxBufCPU[6] = utils::evalChecksum(TxBufCPU, 6);
                                    TxSizeCPU=7;
                                    RxSizeCPU=22;
                                    TxCurrentCPU=0;
                                    break;


                            case ComCommandRequest_RestartCPU_req:
                                    TxBufCPU[0] = charHeaderPacket;
                                    TxBufCPU[1] = CommandCPURestart;
                                    TxBufCPU[2] = 4;
                                    TxBufCPU[3] = utils::evalChecksum(TxBufCPU, 3);
                                    TxSizeCPU=4;
                                    RxSizeCPU=4;
                                    TxCurrentCPU=0;
                                    break;


                            case ComCommandRequest_WriteConfigFile_req:
                                    TxBufCPU[0] = charHeaderPacket;
                                    TxBufCPU[1] = CommandCPUWriteConfigFile;
                                    TxBufCPU[2] = 70;
                                    TxBufCPU[3] = myFileArray_index/ConfigFile_blockDim;
                                    TxBufCPU[4] = ConfigFileSize/ConfigFile_blockDim;
                                    for (i=0; i<ConfigFile_blockDim; i++) {TxBufCPU[5+i]=myFileArray[myFileArray_index]; myFileArray_index++; }
                                    TxBufCPU[69] = utils::evalChecksum(TxBufCPU, 69);
                                    TxSizeCPU=70;
                                    RxSizeCPU=6;
                                    TxCurrentCPU=0;
                                    break;


                            case ComCommandRequest_ReadConfigFile_req:
                                    TxBufCPU[0] = charHeaderPacket;
                                    TxBufCPU[1] = CommandCPUReadConfigFile;
                                    TxBufCPU[2] = 6;
                                    TxBufCPU[3] = myFileArray_index/ConfigFile_blockDim;
                                    TxBufCPU[4] = 0;
                                    TxBufCPU[5] = utils::evalChecksum(TxBufCPU, 5);
                                    TxSizeCPU=6;
                                    RxSizeCPU=70;
                                    TxCurrentCPU=0;
                                    break;


                            case ComCommandRequest_ReadAudit_req:
                                    TxBufCPU[0] = charHeaderPacket;
                                    TxBufCPU[1] = CommandCPUReadAudit;
                                    TxBufCPU[2] = 6;
                                    TxBufCPU[3] = 0;
                                    TxBufCPU[4] = 0;
                                    TxBufCPU[5] = utils::evalChecksum(TxBufCPU, 5);
                                    TxSizeCPU=6;
                                    RxSizeCPU=70;
                                    TxCurrentCPU=0;
                                    break;
                        }


                        serialCPU->readAll();
                        DEBUG_COMM_MSG (TxBufCPU, 0, TxSizeCPU, true);
                        for (i=0; i<TxSizeCPU;i++){ QarrayTX[i]=TxBufCPU[i];}
                        serialCPU->write(QarrayTX, TxSizeCPU);
                        serialCPU->flush();

                        ComStatus=ComStatus_Rx;
                        Com_NumTimeoutChar=0;
                        RxCurrentCPU=0;
                        break;



        case ComStatus_Rx:
                    {
                        QCoreApplication::processEvents();
                        num_byte_received = serialCPU->bytesAvailable();

                        if ( num_byte_received == 0 )
                        {
                            Com_NumTimeoutChar++;
                            if ((Com_NumTimeoutChar * TIMER_INTERVAL_MSEC) >= Com_MAX_TIMEOUT_CHAR_MSEC)
                            {

                                if (ComCommandRequest==ComCommandRequest_WriteConfigFile_req)
                                {
                                    ConfigFileOperation_status=ConfigFileOperation_status_Write_endKO;
                                    ConfigFileOperation_errorCode=1;
                                    ComCommandRequest=ComCommandRequest_CheckStatus_req;
                                }
                                if (ComCommandRequest==ComCommandRequest_ReadConfigFile_req)
                                {
                                    ConfigFileOperation_status=ConfigFileOperation_status_Read_endKO;
                                    ConfigFileOperation_errorCode=1;
                                    ComCommandRequest=ComCommandRequest_CheckStatus_req;
                                }
                                if (ComCommandRequest==ComCommandRequest_ReadAudit_req)
                                {
                                    ConfigFileOperation_status=ConfigFileOperation_status_Audit_endKO;
                                    ConfigFileOperation_errorCode=1;
                                    ComCommandRequest=ComCommandRequest_CheckStatus_req;
                                }



                                DEBUG_MSG ("ComError (rx)");
                                ComStatus=ComStatus_Error;
                                break;
                            }
                            break;
                        }


                        while(serialCPU->bytesAvailable()!=0)
                        {
                            serialCPU->getChar((char*)&RxBufCPU[RxCurrentCPU]);
                            RxCurrentCPU++;
                        }

                        Com_NumTimeoutChar=0;
                        if (RxBufCPU[0]!=charHeaderPacket)
                        {
                            RxCurrentCPU=0;

                            Com_NumTimeoutChar++;
                            break;
                        }
                        if (RxCurrentCPU >= 2)
                        {
                            if(RxBufCPU[2]<MaxLenBufferComCPU_Rx) RxSizeCPU = RxBufCPU[2];
                        }
                        if (RxCurrentCPU >= RxSizeCPU)
                        {
                            ComStatus=ComStatus_HandleReply;
                            goto KeepOnHandleReply;
                        }
                    }
                    break;

        case ComStatus_HandleReply:
                    KeepOnHandleReply:
                    {
                        if ( RxSizeCPU==0 || utils::evalChecksum(RxBufCPU, RxSizeCPU-1) != RxBufCPU[RxSizeCPU-1])
                        {
                            DEBUG_MSG("ComError 2");
                            ComStatus=ComStatus_Error;
                            break;
                        }


                        DEBUG_COMM_MSG (RxBufCPU, 0, RxSizeCPU, false);

                        switch(RxBufCPU[1])
                        {
                            default:
                                Com_NumTimeoutChar = 0;
                                ComStatus = ComStatus_Error_WaitRestart;
                                break;

                            // la cpu mi ha mandato ilsuo stato
                            case CommandCPUCheckStatus:
                            case CommandCPUCheckStatus_Unicode:
                                {
                                    unsigned char isMultilangage = 0;
                                    MsgLcdCPUImportanceLevel = 0xff;
                                    userCurrentCredit[0] = 0;

                                    if (protocol_version >= 1)
                                    {
                                        unsigned char z = 11 + MAXLEN_MSG_LCD;
                                        if (RxBufCPU[1] == CommandCPUCheckStatus_Unicode)
                                            z+=MAXLEN_MSG_LCD;
                                        z+=6;

                                        beepSelezioneLenMSec = RxBufCPU[z+1];
                                        beepSelezioneLenMSec *= 256;
                                        beepSelezioneLenMSec += RxBufCPU[z];
                                        beepSelezioneLenMSec *= 100; //trasforma in msec
                                        z+=2;

                                        //DEBUG_MSG("protocol_version:%d", protocol_version);


                                        if (protocol_version >= 2)
                                        {
                                            //DEBUG_MSG("%02X %02X %02X", RxBufCPU[z], RxBufCPU[z+1], RxBufCPU[z+2]);

                                            //un byte per indicare se la CPU sta usando il suo linguaggio oppure sta usando il linguaggio "ml" (ovvero manda i messaggi con @)
                                            switch (RxBufCPU[z++])
                                            {
                                            default:
                                            case '0':
                                                lang_clearErrorCode (language);
                                                z+=2;
                                                break;

                                            case '1':
                                                {
                                                    //cpu usa un linguaggio custom, vediamo che lingua vuole
                                                    isMultilangage = 1;
                                                    char language_requested[4];
                                                    language_requested[0] = RxBufCPU[z++];
                                                    language_requested[1] = RxBufCPU[z++];



                                                    //se necessario, cambio lingua
                                                    const char *curLang = lang_getCurLanguage (language);
                                                    if (curLang[0] != language_requested[0] || curLang[1] != language_requested[1])
                                                    {
                                                        language_requested[2] = 0x00;
                                                        lang_open (language, language_requested);
                                                    }
                                                }
                                                break;
                                            }


                                            if (protocol_version >= 3)
                                            {
                                                //1 byte per indicare importanza del msg di CPU (0=poco importante, 1=importante)
                                                //8 byte stringa con l'attuale credito
                                                MsgLcdCPUImportanceLevel = RxBufCPU[z++];
                                                memcpy (userCurrentCredit, &RxBufCPU[z], 8);
                                                userCurrentCredit[8] = 0;
                                                z += 8;
                                            }

                                        } // if (protocol_version >= 2)
                                    } //if (protocol_version >= 1)

                                    VMCstate=RxBufCPU[3];
                                    VMCerrorCode=RxBufCPU[4];
                                    VMCerrorType=RxBufCPU[5];


                                    CupAbsentStatus_flag = RxBufCPU[9] & 0x08;
                                    bShowDialogStopSelezione = RxBufCPU[9] & 0x10;

                                    /*  GIX 2018 05 04
                                        Abbiamo deciso che la GPU deve avere un modo per sapere se la CPU sta preparando
                                        una bevanda e, se si, se è in attesa che i sistemi di pagamento facciano il loro mestiere.
                                        Usiamo i bit 0x20 e 0x40 in questo modo:
                                            se == 0  => la CPU è una versione "vecchia" che non supporta questa nuova procedura
                                                        per cui anche la GPU deve funzionare come faceva prima

                                            se == 0x01  =>  la CPU è nuova, 0x01 è il suo stato di default e vuol dire "non sto facendo nulla"
                                            se == 0x02  =>  la CPU ha capito che deve preparare una bevanda, è in attesa che i sistemi di pagamento rispondano
                                            se == 0x03  =>  la CPU ha dato l'OK alla preparazione (equivalente di BEVANDA IN PREP)
                                    */
                                    statoPreparazioneBevanda = (eStatoPreparazioneBevanda)((RxBufCPU[9] & 0x60) >> 5);

                                    unsigned char Selection_CPU_current = RxBufCPU[10];

                                    if (getButtonKeyNum() != 0)
                                    {
                                        if(VMCstate != VMCSTATE_PROGRAMMAZIONE)
                                        {
                                            if (VMCstate == VMCSTATE_PREPARAZIONE_BEVANDA && Selection_CPU_current == getButtonKeyNum())
                                            {
                                                DEBUG_MSG ("set btnKeyNum to zero (bevanda in prep)");
                                                setButtonKeyNum(0);
                                            }
                                        }
                                    }

                                    //32 caratteri ascii oppure 64 bytes unicode
                                    k=0;
                                    z=11;
                                    QChar firstGoodChar = ' ';
                                    for (i=0;i<MAXLEN_MSG_LCD;i++)
                                    {
                                        if(RxBufCPU[1]==CommandCPUCheckStatus_Unicode)
                                        {
                                            MsgLcdCPU[k]=RxBufCPU[z]+RxBufCPU[z+1]*256;
                                            z+=2;
                                        }
                                        else
                                        {
                                            MsgLcdCPU[k]=RxBufCPU[z];
                                            z++;
                                        }

                                        if (MsgLcdCPU[k] != ' ' && firstGoodChar==' ')
                                            firstGoodChar = MsgLcdCPU[k];
                                        k++;

                                        if (k==16)
                                        {
                                            if (!isMultilangage || firstGoodChar!='@')
                                                MsgLcdCPU[k++]=' ';
                                        }
                                    }
                                    MsgLcdCPU[k]=0;


                                    //1 bit per ogni selezione per indicare se la selezione è disponibile o no
                                    //Considerando che NumMaxSelections=48, dovrebbero servire 6 byte
                                    //ATTENZIONE che bit==0 significa che la selezione è OK, bit==1 significa KO
                                    if( (RxSizeCPU>(z+2)) && (VMCstate!=VMCSTATE_INITIAL_CHECK) && (VMCstate!=VMCSTATE_ERROR) )
                                    {
                                        u8 mask = 0x01;
                                        u8 anythingChanged = 0;
                                        u8 atLeastOneIsOK = 0;
                                        for (u8 i=0; i<NUM_MAX_SELECTIONS; i++)
                                        {
                                            u8 isSelectionOK = 1;
                                            if( (RxBufCPU[z] & mask) != 0)
                                                isSelectionOK = 0;

                                            if (isSelectionOK)
                                            {
                                                atLeastOneIsOK = 1;
                                                if (selAvailability.isAvail(i+1)== false)
                                                {
                                                    anythingChanged = 1;
                                                    selAvailability.setAsAvail(i+1);
                                                }
                                            }
                                            else
                                            {
                                                if (selAvailability.isAvail(i+1) == true)
                                                {
                                                    anythingChanged=1;
                                                    selAvailability.setAsNotAvail(i+1);
                                                }
                                            }

                                            if (mask == 0x80)
                                            {
                                                mask = 0x01;
                                                z++;
                                            }
                                            else
                                                mask <<= 1;
                                        }

                                        if (anythingChanged)
                                            mainWindow->guibridgeEvent_selectionAvailabilityUpdated(NUM_MAX_SELECTIONS, selAvailability);

                                        if (!atLeastOneIsOK)
                                        {
                                            //se nemmeno una delle selezioni è disponibile, certamente c'è da mostrare qualche messaggio
                                            //della CPU, indipendentemente dal suo livello di importanza!
                                            MsgLcdCPUImportanceLevel = 0xff;
                                        }
                                    }

                                    //se non c'è nemmeno una selezione disponibile, mostro sempre e cmq il msg di CPU anche se non fosse "importante"
                                    if (selAvailability.areAllNotAvail())
                                        MsgLcdCPUImportanceLevel = 0xff;

                                    if (flag_initial_param_received==0)
                                        ComCommandRequest = ComCommandRequest_InitialParam_req;
                                    ComStatus=ComStatus_ReplyOK;
                                    goto KeepOnReplyOK;
                                }
                                break;

                            case CommandCPUInitialParam_C:
                            /*	GIX 2018-12-17
                                byte		significato
                                0			#
                                1			C
                                2			lunghezza

                                3			anno
                                4			mese
                                5			giorno

                                6			hh
                                7			mm
                                8			ss

                                9			versione sw	8 caratteri del tipo "1.4 WIP"
                                10			versione sw
                                11			versione sw
                                12			versione sw
                                13			versione sw
                                14			versione sw
                                15			versione sw
                                16			versione sw

                                17			98 btyes composti da 49 prezzi ciascuno da 2 bytes
                                ....

                                Da qui in poi sono dati nuovi, introdotti a dicembre 2018

                                115			versione protocollo. Inizialmente = 1, potrebbe cambiare in futuro

                                116			ck
                            */
                                {
                                    int Date_year = RxBufCPU[3]+2000;
                                    int Date_month = RxBufCPU[4];
                                    int Date_dayOfMonth = RxBufCPU[5];
                                    int Date_hour = RxBufCPU[6];
                                    int Date_min = RxBufCPU[7];
                                    int Date_sec = RxBufCPU[8];
                                    CPU_version = QString::fromLocal8Bit((const char *)&(RxBufCPU[9]), 8);
                                    k=17;
                                    if (RxSizeCPU>24)
                                    {
                                        for (i=0; i<NUM_MAX_SELECTIONS; i++)
                                        {
                                            Prices[i] = (unsigned int)RxBufCPU[k] + 256*RxBufCPU[k+1];
                                            k=k+2;
                                        }

                                        mainWindow->guibridgeEvent_selectionPricesUpdated (NUM_MAX_SELECTIONS, Prices);
                                    }

                                    if (RxBufCPU[2] >= 117)
                                    {
                                        protocol_version = RxBufCPU[115];
                                    }



                                    flag_initial_param_received=1;

                                    if ((Date_year*Date_month*Date_dayOfMonth)==0)
                                    {
                                        Date_year=2015; Date_month=3; Date_dayOfMonth=15;
                                        Date_hour=15; Date_min=1; Date_sec=0;
                                    }


                                    char s[256];
                                    sprintf(s, "date -u %02d%02d%02d%02d%04d.%02d", Date_month, Date_dayOfMonth, Date_hour, Date_min, Date_year, Date_sec);
                                    system(s);
                                    system("hwclock -w");

                                    ComCommandRequest=ComCommandRequest_CheckStatus_req;
                                    ComStatus=ComStatus_ReplyOK;
                                }
                                goto KeepOnReplyOK;
                                break;



                            case CommandCPUWriteConfigFile:
                                {
                                    if (myFileArray_index >= ConfigFileSize)
                                    {
                                        ConfigFileOperation_status=ConfigFileOperation_status_Write_endOK;
                                        ComCommandRequest=ComCommandRequest_CheckStatus_req;
                                    }
                                    ComStatus=ComStatus_ReplyOK;
                                }
                                goto KeepOnReplyOK;
                                break;

                            case CommandCPUReadConfigFile:
                                {
                                    for (i=0; i<ConfigFile_blockDim; i++) { myFileArray[myFileArray_index]=RxBufCPU[5+i]; myFileArray_index++; }
                                    if (myFileArray_index==ConfigFileSize)
                                    {
                                        ConfigFileOperation_status=ConfigFileOperation_status_Read_endOK;
                                        ComCommandRequest=ComCommandRequest_CheckStatus_req;
                                    }
                                    ComStatus=ComStatus_ReplyOK;
                                }
                                goto KeepOnReplyOK;
                                break;

                            case CommandCPUReadAudit:
                                {
                                    for (i=0; i<(RxSizeCPU-6); i++) { myAuditArray[myFileArray_index]=RxBufCPU[5+i]; myFileArray_index++; }
                                    if (RxBufCPU[3]!=0)
                                    {
                                        ConfigFileOperation_status=ConfigFileOperation_status_Audit_endOK;
                                        ComCommandRequest=ComCommandRequest_CheckStatus_req;
                                    }
                                    ComStatus=ComStatus_ReplyOK;
                                }
                                goto KeepOnReplyOK;
                                break;
                        }
                    }
                    break;

        case ComStatus_ReplyOK:
                    KeepOnReplyOK:
                        ComStatus=ComStatus_Idle;
                        Com_NumTotTimeoutCommand=0;
                        break;


        case ComStatus_Error:
                    if(Com_NumTotTimeoutCommand < Com_MAX_TOT_TIMEOUT_COMMAND)
                        Com_NumTotTimeoutCommand++;
                    else
                        VMCstate = VMCSTATE_COM_ERROR;
                    Com_NumTimeoutChar=0;
                    ComStatus = ComStatus_Error_WaitRestart;
                    break;

        default:
            break;

        case ComStatus_Error_WaitRestart:
                    Com_NumTimeoutChar++;
                    if ((Com_NumTimeoutChar*TIMER_INTERVAL_MSEC)>=500)
                    {
                        ComCommandRequest = ComCommandRequest_CheckStatus_req;
                        ComStatus = ComStatus_Idle;
                    }
                    break;
    }
}

void MainWindow::debug_showTreeViewOfInstalledFont()
{
    QFontDatabase database;
    ui->dbg_treeWidget->setColumnCount(2);
    ui->dbg_treeWidget->setHeaderLabels(QStringList() << "Font" << "Smooth Sizes");
    ui->dbg_treeWidget->raise();

    const QStringList fontFamilies = database.families();

    for (int i=0; i<fontFamilies.count(); i++)
    //for (const QString &family : fontFamilies)
    {
        const QString &family = fontFamilies[i];
        QTreeWidgetItem *familyItem = new QTreeWidgetItem(ui->dbg_treeWidget);
        familyItem->setText(0, family);

        const QStringList fontStyles = database.styles(family);
        //for (const QString &style : fontStyles)
        for (int i2=0; i2<fontStyles.count(); i2++)
        {
            const QString &style = fontStyles[i2];
            QTreeWidgetItem *styleItem = new QTreeWidgetItem(familyItem);
            styleItem->setText(0, style);

            QString sizes;
            const QList<int> smoothSizes = database.smoothSizes(family, style);
            //for (int points : smoothSizes)
            for (int t=0; t<smoothSizes.count(); t++)
            {
                int points = smoothSizes[t];
                sizes += QString::number(points) + ' ';
            }
            styleItem->setText(1, sizes.trimmed());
        }
    }
}
