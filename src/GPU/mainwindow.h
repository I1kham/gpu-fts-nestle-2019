#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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
    explicit                MainWindow (const sGlobal *glob);
                            ~MainWindow();

private slots:
    void                    timerInterrupt();


private:
    const sGlobal           *glob;
    Ui::MainWindow          *ui;
    QTimer                  *timer;
};

#endif // MAINWINDOW_H
