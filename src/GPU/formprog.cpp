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
FormProg::FormProg(QWidget *parent, const sGlobal *globIN) :
        QDialog(parent),
        ui(new Ui::FormProg)
{
    glob = globIN;
    retCode = 0;
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
    cpubridge::ask_CPU_PROGRAMMING_CMD (glob->subscriber, 0, cpubridge::eCPUProgrammingCommand_enterProg);

    retCode = 0;
    this->show();
}

//*******************************************
int FormProg::onTick()
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
            u8 vmcErrorCode, vmcErrorType;
            cpubridge::translateNotify_CPU_STATE_CHANGED (msg, &vmcState, &vmcErrorCode, &vmcErrorType);
            if (vmcState != cpubridge::eVMCState_PROGRAMMAZIONE)
                retCode = 1;
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
            ui->labelStatus->setText(QString(msgCPU, -1));
        }
        break;
    }
}




//************************************************************
void FormProg::priv_updateLabelVersion()
{
    char str[32];
    sprintf (str, "%d.%d.%d", GPU_VERSION_MAJOR, GPU_VERSION_MINOR, GPU_VERSION_BUILD);
    ui->labelVersion->setText("GPU version: " + QString(str) + "  -  CPU:" + glob->cpuVersion);
}

//************************************************************
void FormProg::on_buttonB1_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 1); }
void FormProg::on_buttonB2_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 2); }
void FormProg::on_buttonB3_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 3); }
void FormProg::on_buttonB4_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 4); }
void FormProg::on_buttonB5_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 5); }
void FormProg::on_buttonB6_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 6); }
void FormProg::on_buttonB7_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 7); }
void FormProg::on_buttonB8_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 8); }
void FormProg::on_buttonB9_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 9); }
void FormProg::on_buttonB10_pressed()       { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 10); }

