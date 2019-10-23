#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "header.h"
#include <QMainWindow>
#include <QWidget>
#include <QMessageBox>
#include "formboot.h"
#include "FormBrowser.h"
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

private:
    enum eForm
    {
        eForm_main = 0,
        eForm_boot,
        eForm_browser,
        eForm_prog
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
    void                    priv_showForm (eForm w);
    void                    priv_start();
    void                    priv_onCPUBridgeNotification (rhea::thread::sMsg &msg);
    void                    priv_setText (const char *s);
    void                    priv_onTick();
    void                    priv_scheduleFormChange(eForm w);

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
    FormBrowser             *frmBrowser;
    FormProg                *frmProg;
};

#endif // MAINWINDOW_H
