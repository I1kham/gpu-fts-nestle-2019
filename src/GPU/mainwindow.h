#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QMessageBox>



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
    explicit                MainWindow ();
                            ~MainWindow();

private slots:
    void                    timerInterrupt();

private:
    void                    priv_loadGUIFirstPage();

private:
    Ui::MainWindow          *ui;
    bool                    isInterruptActive;
    QTimer                  *timer;
};

#endif // MAINWINDOW_H
