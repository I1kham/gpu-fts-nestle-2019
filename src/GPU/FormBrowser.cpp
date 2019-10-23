#include "header.h"
#include <QTimer>
#include <QWebSettings>
#include "FormBrowser.h"
#include "ui_FormBrowser.h"
#include "history.h"



//********************************************************************************
FormBrowser::FormBrowser(QWidget *parent, sGlobal *globIN) :
    QDialog(parent),
    ui(new Ui::FormBrowser)
{
    glob = globIN;
    retCode = 0;

    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);


    //Settaggi del browser
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::AutoLoadImages, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);

    //espando la webView a tutto schermo
    ui->webView->setMouseTracking(false);
    ui->webView->raise();
    ui->webView->settings()->setAttribute(QWebSettings::PluginsEnabled,true);
    ui->webView->settings()->setAttribute(QWebSettings::AutoLoadImages, true);
    ui->webView->settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);
    ui->webView->setEnabled(false);

#ifdef _DEBUG
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    ui->webView->setEnabled(true);
    ui->webView->setFocus();
#endif
}

//********************************************************************************
FormBrowser::~FormBrowser()
{
    delete ui;
}

//*******************************************
void FormBrowser::showMe()
{
    retCode = 0;
    this->show();

    //carico la GUI nel browser
    char s[1024];
    sprintf_s (s, sizeof(s), "file://%s/web/startup.html", glob->current_GUI);
    ui->webView->load(QUrl(s));
    ui->webView->setFocus();

}

//********************************************************************************
int FormBrowser::onTick()
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
void FormBrowser::priv_onCPUBridgeNotification (rhea::thread::sMsg &msg)
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
        }
        break;

    case CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS:
        {
            cpubridge::eRunningSelStatus s = cpubridge::eRunningSelStatus_finished_KO;
            cpubridge::translateNotify_CPU_RUNNING_SEL_STATUS (msg, &s);
            if (s == cpubridge::eRunningSelStatus_finished_OK)
                History::incCounterSelezioni();
        }
        break;

    case CPUBRIDGE_NOTIFY_BTN_PROG_PRESSED:
        //l'utente ha premuto il btn PROG, devo andare in programmazione
        retCode = 1;
        break;
    }
}


