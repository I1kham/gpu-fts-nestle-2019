#ifndef FORMBOOT_H
#define FORMBOOT_H
#include <QDialog>

namespace Ui {
class FormBoot;
}

class FormBoot : public QDialog
{
    Q_OBJECT

public:
    explicit FormBoot(QWidget *parent = 0);
    ~FormBoot();

public:
    Ui::FormBoot *ui;

private:
    bool    bBtnStartVMCEnabled;

private:
    bool    priv_langCopy (const QString &srcFolder, const QString &dstFolder, long timeToWaitDuringCopyFinalizingMSec) const;
    void    foreverDisableBtnStartVMC();

private slots:
    void on_buttonStart_clicked();
    void on_buttonWriteSettings_clicked();
    void on_buttonTouchCalib_clicked();
    void timerInterrupt();
    void on_buttonReadSettings_clicked();
    void on_buttonWriteCPU_clicked();
    void on_buttonWriteGUI_clicked();
    void on_buttonWriteManual_clicked();
    void on_buttonReadGUI_clicked();
    void on_buttonReadAudit_clicked();
    void on_btnWriteLang_clicked();
    void on_btnReadLang_clicked();
};

#endif // FORMBOOT_H
