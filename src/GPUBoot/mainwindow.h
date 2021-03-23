#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qscrollbar.h>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnUpdateSMU_clicked();
    void on_lbFileList_doubleClicked(const QModelIndex &index);

    void on_btnCloseErrorLog_clicked();

    void on_btnStartVMC_clicked();

private:
    void        priv_fileListPopulate ();
    bool        priv_unpackMH210(const QString &fullFilePathAndName);
    void        priv_log (const QString &text);
    void        priv_on_unpackMH210_finished();
    void        priv_setMessageOKWithColor (const QString &msg, const char *bgColor, const char *textColor);
    void        priv_setMessageOK (const QString &msg);
    void        priv_setMessageKO (const QString &msg);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
