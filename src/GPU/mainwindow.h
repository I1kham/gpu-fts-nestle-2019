#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMessageBox>
#include "formprog.h"
#include "formboot.h"
#include "formresetgrncounter.h"
#include "lang.h"
#include "../rheaCommonLib/rheaThread.h"
#include "SelectionAvailiability.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum eSelectionReqStatus
    {
        eSelectionReqStatus_waitingCPU = 1,
        eSelectionReqStatus_preparing = 2,
        eSelectionReqStatus_aborted = 3,
        eSelectionReqStatus_finished = 4,
        eSelectionReqStatus_preparing_canUseStopBtn = 5,
        eSelectionReqStatus_unknown = 0xff
    };

public:
    explicit        MainWindow(const HThreadMsgR hQMessageFromWebserver, const HThreadMsgW hQMessageToWebserver);
                    ~MainWindow();

    void            guibridgeEvent_selectionAvailabilityUpdated (int numSel, const SelectionAvailability &selAvailList) const;
    void            guibridgeEvent_selectionPricesUpdated (int numSel, const unsigned int *priceList) const;
    void            guibridgeEvent_selectionReqStatus (eSelectionReqStatus status) const;
    void            guibridgeEvent_sendCPUMessage (const QString &msg, u8 importanceLevel) const;
    void            debug_showTreeViewOfInstalledFont();

public:
    FormProg                myFormProg;

private:
    Ui::MainWindow          *ui;
    bool                    isInterruptActive;
    FormBoot                myFormBoot;
    FormResetGrnCounter     *dialogAButton;
    int                     formStatus;
    long                    timeFormStatusChangedMSec;
    char                    bBevandaInPreparazione;
    int                     debug_ct_pageStandBy;
    long                    debug_lastTimePageStandByWasShown;
    sLanguage               language;
    QString                 lastReceivedCPUMsg;
    QString                 currentShownCPUMsg;
    HThreadMsgR             hQMessageFromWebserver;
    HThreadMsgW             hQMessageToWebserver;
    u64                     timeToSendCPUMsgToGUI_MSec;

private slots:
    void            timerInterrupt();

private:
    void            priv_loadGUIFirstPage();

    int             getFormStatus()                                                     { return formStatus; }
    void            setFormStatus (int i, const char *who);

    void            priv_showDiaglogResetGroundCounter();
    void            priv_hideDiaglogResetGroundCounter();
    void            priv_showDiaglogCleanMilker();
    void            priv_hideDiaglogCleanMilker();

    void            priv_handleFormStatus_NORMAL();
    void            priv_handleFormStatus_SELECTION_RUNNING();
    void            priv_onSelezioneTerminataOK();
    void            priv_onSelezioneTerminataKO();
    void            priv_translateCPUMessage (const QChar *cpuMsgBuffer, int sizeofCpuMsgBuffer, unsigned char msgImportanceLevel);

    void            priv_handleGUIBridgeServerMessages();
    void            priv_guibridgeAnswer_selectionAvailabilityList (u16 handlerID, int numSel, const SelectionAvailability &selAvailList) const;
    void            priv_guibridgeAnswer_selectionPricesList (u16 handlerID, int numSel, const unsigned int *priceList) const;
    void            priv_guibridgeAnswer_credit (u16 handlerID, const char *credit) const;};

#endif // MAINWINDOW_H
