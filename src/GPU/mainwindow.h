#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "header.h"
#include <QMainWindow>
#include <QWidget>
#include <QMessageBox>
#include "formboot.h"
#include "formprog.h"
#include "formPreGui.h"
#include "../CPUBridge/CPUBridge.h"


namespace Ui
{
    class MainWindow;
}

/*******************************************************************
 * MainWindow
 *
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit                MainWindow (sGlobal *glob);
                            ~MainWindow();

private slots:
    void                    timerInterrupt();
    void                    on_webView_urlChanged(const QUrl &arg1);

private:
    enum eForm
    {
        eForm_main_syncWithCPU = 0,
        eForm_boot,
        eForm_specialActionBeforeGUI,
        eForm_main_showBrowser,
        eForm_oldprog_legacy,

        eForm_newprog = 100,
        eForm_newprog_lavaggioSanitario = 101,
        eForm_newprog_lavaggioMilker = 102,
        eForm_newprog_descaling = 103,
		eForm_newprog_dataAudit = 104
    };

private:
    void                    priv_loadURL (const char *url);
    void                    priv_loadURLMenuProg (const char *paramsInGet);
    bool                    priv_shouldIShowFormPreGUI();
    void                    priv_showForm (eForm w);
    void                    priv_start();
    void                    priv_syncWithCPU_onCPUBridgeNotification (rhea::thread::sMsg &msg);
    void                    priv_addText (const char *s);
    void                    priv_syncWithCPU_onTick();
    void                    priv_scheduleFormChange(eForm w);
    eRetCode                priv_showBrowser_onTick();
    void                    priv_showBrowser_onCPUBridgeNotification (rhea::thread::sMsg &msg);
    eRetCode                priv_showNewProgrammazione_onTick();
    void                    priv_showNewProgrammazione_onCPUBridgeNotification (rhea::thread::sMsg &msg);
    void                    priv_showLockedPanel (bool b);

private:
    struct sSyncWithCPU
    {
        cpubridge::eVMCState vmcState;
        u64                 nextTimeoutAskCPUStateMSec;
        u64                 esapiTimeoutMSec;
        u8                  stato;

        void            reset() { stato=0; nextTimeoutAskCPUStateMSec=0; vmcState = cpubridge::eVMCState::COMPATIBILITY_CHECK; }
    };

private:
    sGlobal                 *glob;
    Ui::MainWindow          *ui;
    QTimer                  *timer;
    bool                    isInterruptActive;
    sSyncWithCPU            syncWithCPU;
    cpubridge::sCPUVMCDataFileTimeStamp myTS;
    eForm                   currentForm, nextForm;
    FormBoot                *frmBoot;
    FormProg                *frmProg;
    FormPreGui              *frmPreGUI;
    eRetCode                retCode;
    u64                     nextTimeAskForCPULockStatus_msec;
};

#endif // MAINWINDOW_H
