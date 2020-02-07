#include "header.h"
#include "formprog.h"
#include "ui_formprog.h"
#include "Utils.h"
#include <QWidget>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QUrl>
#include <QTimer>

extern QString CPU_version;


//************************************************************
FormProg::FormProg(QWidget *parent, sGlobal *globIN) :
        QDialog(parent),
        ui(new Ui::FormProg)
{
    glob = globIN;
    retCode = eRetCode_none;
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    priv_updateLabelVersion();
    ui->labelVersion->raise();
    ui->labelVersion->setAlignment(Qt::AlignRight);

    //ui->labelStatus->setStyleSheet("QLabel { background-color: #808080; color:#fff; }");
    ui->labelStatus->setText("");
    //utils::getRightFontForLanguage (theFont, 20, iso2LettersLanguageCode);
    utils::getRightFontForLanguage (theFont, 20, "GB");
    ui->labelStatus->setFont (theFont);
    ui->labelStatus->raise();

    cpubridge::ask_CPU_QUERY_LCD_MESSAGE(glob->subscriber, 0);
}

//************************************************************
FormProg::~FormProg()
{
    delete ui;
}

//*******************************************
void FormProg::showMe()
{
    cpubridge::ask_CPU_PROGRAMMING_CMD (glob->subscriber, 0, cpubridge::eCPUProgrammingCommand_enterProg, NULL, 0);

    retCode = eRetCode_none;
    this->show();
}

//*******************************************
eRetCode FormProg::onTick()
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

/**************************************************************************
 * priv_onCPUBridgeNotification
 *
 * E' arrivato un messaggio da parte di CPUBrdige sulla msgQ dedicata (ottenuta durante la subscribe di this a CPUBridge).
 */
void FormProg::priv_onCPUBridgeNotification (rhea::thread::sMsg &msg)
{
    const u16 handlerID = (msg.paramU32 & 0x0000FFFF);
    assert (handlerID == 0);

    const u16 notifyID = (u16)msg.what;
    switch (notifyID)
    {
    case CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED:
        {
            cpubridge::eVMCState vmcState;
            u8 vmcErrorCode = 0, vmcErrorType = 0;
            u16 flag1 = 0;
            cpubridge::translateNotify_CPU_STATE_CHANGED (msg, &vmcState, &vmcErrorCode, &vmcErrorType, &flag1);

            if (0 != (flag1 & cpubridge::sCPUStatus::FLAG1_READY_TO_DELIVER_DATA_AUDIT))
                glob->bCPUEnteredInMainLoop=1;

            //quando la CPU cambia di stato e diventa DISP o INI_CHECK, io torno al browser
            if (vmcState == cpubridge::eVMCState_DISPONIBILE || vmcState==cpubridge::eVMCState_INITIAL_CHECK)
                retCode = eRetCode_gotoFormBrowser;
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
            ui->labelStatus->setText(QString(msgCPU, -1));
        }
        break;
    }
}




//************************************************************
void FormProg::priv_updateLabelVersion()
{
    ui->labelVersion->setText("GPU version: " GPU_VERSION "  -  CPU:" + QString(glob->cpuVersion));
}

//************************************************************
void FormProg::on_buttonB1_pressed()        { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 1); }
void FormProg::on_buttonB2_pressed()        { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 2); }
void FormProg::on_buttonB3_pressed()        { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 3); }
void FormProg::on_buttonB4_pressed()        { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 4); }
void FormProg::on_buttonB5_pressed()        { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 5); }
void FormProg::on_buttonB6_pressed()        { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 6); }
void FormProg::on_buttonB7_pressed()        { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 7); }
void FormProg::on_buttonB8_pressed()        { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 8); }
void FormProg::on_buttonB9_pressed()        { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 9); }
void FormProg::on_buttonB10_pressed()       { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 10); }


void FormProg::on_buttonB1_released()   { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 0); }
void FormProg::on_buttonB2_released()   { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 0); }
void FormProg::on_buttonB3_released()   { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 0); }
void FormProg::on_buttonB4_released()   { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 0); }
void FormProg::on_buttonB5_released()   { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 0); }
void FormProg::on_buttonB6_released()   { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 0); }
void FormProg::on_buttonB7_released()   { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 0); }
void FormProg::on_buttonB8_released()   { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 0); }
void FormProg::on_buttonB9_released()   { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 0); }
void FormProg::on_buttonB10_released()  { cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM (glob->subscriber, 0); }
