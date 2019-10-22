#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "header.h"
#include <QMainWindow>
#include <QWidget>
#include <QMessageBox>
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
    enum eStato
    {
        eStato_running = 0,
        eStato_sync_1_queryCpuStatus,
        eStato_sync_1_wait,
        eStato_sync_2_queryVMCSettingTS,
        eStato_sync_2_wait,
        eStato_sync_3_downloadVMCSetting,
        eStato_sync_3_wait
    };

private:
    void                    priv_start();
    void                    priv_onCPUBridgeNotification (rhea::thread::sMsg &msg);
    void                    priv_setText (const char *s);

private:
    sGlobal                 *glob;
    Ui::MainWindow          *ui;
    QTimer                  *timer;
    eStato                  stato;
    u64                     statoTimeout;
    bool                    isInterruptActive;
    cpubridge::sCPUVMCDataFileTimeStamp myTS;
};

#endif // MAINWINDOW_H
