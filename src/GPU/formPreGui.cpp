#include "formPreGui.h"
#include "ui_formPreGui.h"
#include "../rheaAppLib/rheaAppUtils.h"

//************************************************************
FormPreGui::FormPreGui(QWidget *parent, sGlobal *glob) :
    QDialog(parent), ui(new Ui::FormPreGui)
{
    this->glob = glob;
    retCode = eRetCode_none;
    timeToAutoCloseMSec = 0;
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    ui->labMsg->setVisible(false);
}

//************************************************************
FormPreGui::~FormPreGui()
{
    delete ui;
}

//*******************************************
void FormPreGui::showMe (u16 groundCounterLimit, bool bShowBtnCleaMilker)
{
    retCode = eRetCode_none;
    cpubridge::ask_CPU_QUERY_STATE(glob->subscriber, 0);
    timeToAutoCloseMSec = rhea::getTimeNowMSec() +10000;
    priv_enableButtonClose (true);

    this->groundCounterLimit = groundCounterLimit;
    this->bShowBtnCleanMilker = bShowBtnCleaMilker;

    priv_enableButtonResetGrndCounter( (groundCounterLimit > 0) );
    priv_enableButtonCleanMilker(bShowBtnCleaMilker);
    this->priv_showMsg(NULL);
    this->show();

    if (this->groundCounterLimit == 0 && !bShowBtnCleaMilker)
        this->on_btnClose_clicked();
}

//************************************************************
void FormPreGui::priv_setButtonStyle (QPushButton *btn, bool bEnabled)
{
    if (bEnabled)
        btn->setStyleSheet ("background-color:#656565; color:#000; border-radius:10px;");
    else
        btn->setStyleSheet ("background-color:#151515; color:#333; border-radius:10px;");
}

//************************************************************
void FormPreGui::priv_enableButtonResetGrndCounter (bool b)
{
    this->ui->btnResetGrndCounter->setEnabled(b);
    priv_setButtonStyle (this->ui->btnResetGrndCounter, b);
}

//************************************************************
void FormPreGui::priv_enableButtonCleanMilker (bool b)
{
    this->ui->btnMilkModuleRInse->setEnabled(b);
    priv_setButtonStyle (this->ui->btnMilkModuleRInse, b);
}

//************************************************************
void FormPreGui::priv_enableButtonClose (bool b)
{
    this->ui->btnClose->setEnabled(b);
    priv_setButtonStyle (this->ui->btnClose, b);
}

//************************************************************
void FormPreGui::priv_updateButtonClose()
{
    if (0 == timeToAutoCloseMSec)
    {
        this->ui->btnClose->setText ("CLOSE");
        return;
    }

    const u64 timeNowMSec = rhea::getTimeNowMSec();
    if (timeNowMSec >= timeToAutoCloseMSec)
    {
        on_btnClose_clicked();
        return;
    }

    char s[128];
    const u64 timeLeftSec = (timeToAutoCloseMSec - timeNowMSec) / 1000;
    sprintf_s (s, sizeof(s), "CLOSE (%d sec)", (u16)timeLeftSec);
    this->ui->btnClose->setText (s);
}


//*******************************************
eRetCode FormPreGui::onTick()
{
    if (retCode != eRetCode_none)
        return retCode;

    priv_updateButtonClose();

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
void FormPreGui::priv_onCPUBridgeNotification (rhea::thread::sMsg &msg)
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
            ui->labCPUStatus->setText (rhea::app::utils::verbose_eVMCState (vmcState));

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
    }
}


//************************************************************
void FormPreGui::priv_showMsg (const char *text)
{
    if (NULL == text)
    {
        ui->labMsg->setVisible(false);
        return;
    }
    if (text[0] == 0x00)
    {
        ui->labMsg->setVisible(false);
        return;
    }
    ui->labMsg->setText (text);
    ui->labMsg->setVisible(true);
}

//************************************************************
void FormPreGui::on_btnClose_clicked()
{
    retCode = eRetCode_gotoFormBrowser;
}

//************************************************************
void FormPreGui::on_btnResetGrndCounter_clicked()
{
    timeToAutoCloseMSec = 0;
    priv_enableButtonResetGrndCounter (false);
    priv_enableButtonCleanMilker(false);
    priv_enableButtonClose (false);

    char s[128];
    sprintf_s (s, sizeof(s), "Resetting ground counter to %d, please wait...", groundCounterLimit);
    priv_showMsg (s);

    cpubridge::ask_CPU_SET_DECOUNTER (glob->subscriber, 0, cpubridge::eCPUProgrammingCommand_decounter_coffeeGround, groundCounterLimit);
    utils::waitAndProcessEvent(1000);

    priv_showMsg ("Done!");
    utils::waitAndProcessEvent(1000);

    showMe (0, this->bShowBtnCleanMilker);
}

//************************************************************
void FormPreGui::on_btnMilkModuleRInse_clicked()
{
    timeToAutoCloseMSec = 0;
    priv_enableButtonResetGrndCounter (false);
    priv_enableButtonCleanMilker(false);
    priv_enableButtonClose (false);
    priv_showMsg ("Starting milker rinsing...");

    cpubridge::ask_CPU_PROGRAMMING_CMD_CLEANING(glob->subscriber, 0, cpubridge::eCPUProgrammingCommand_cleaningType_milker);
    utils::waitAndProcessEvent(1500);

    showMe (groundCounterLimit, false);
}
