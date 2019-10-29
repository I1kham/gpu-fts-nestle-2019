#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "header.h"
#include <QMainWindow>
#include <QWidget>
#include <QMessageBox>
#include "formboot.h"
#include "formprog.h"
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
        eForm_main_showBrowser,
        eForm_oldprog_legacy,
        eForm_newprog
    };

    enum eStato
    {
        eStato_running = 0,
        eStato_sync_1_queryIniParam,
        eStato_sync_1_wait,
        eStato_sync_2_queryCpuStatus,
        eStato_sync_2_wait,
        eStato_sync_3_queryVMCSettingTS,
        eStato_sync_3_wait,
        eStato_sync_4_downloadVMCSetting,
        eStato_sync_4_wait
    };

private:
    void                    priv_loadURL (const char *url);
    void                    priv_showForm (eForm w);
    void                    priv_start();
    void                    priv_syncWithCPU_onCPUBridgeNotification (rhea::thread::sMsg &msg);
    void                    priv_addText (const char *s);
    void                    priv_syncWithCPU_onTick();
    void                    priv_scheduleFormChange(eForm w);
    int                     priv_showBrowser_onTick();
    void                    priv_showBrowser_onCPUBridgeNotification (rhea::thread::sMsg &msg);
    int                     priv_showNewProgrammazione_onTick();
    void                    priv_showNewProgrammazione_onCPUBridgeNotification (rhea::thread::sMsg &msg);

private:
    sGlobal                 *glob;
    Ui::MainWindow          *ui;
    QTimer                  *timer;
    eStato                  stato;
    u64                     statoTimeout;
    bool                    isInterruptActive;
    cpubridge::sCPUVMCDataFileTimeStamp myTS;
    eForm                   currentForm, nextForm;
    FormBoot                *frmBoot;
    FormProg                *frmProg;
    int                     retCode;
};

#endif // MAINWINDOW_H
