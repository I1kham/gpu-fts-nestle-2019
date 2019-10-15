#ifndef FORMBOOT_H
#define FORMBOOT_H
#include <QDialog>

namespace Ui
{
    class FormBoot;
}

/*******************************************************************
 * MainWindow
 *
 */
class FormBoot : public QDialog
{
    Q_OBJECT

public:
    explicit            FormBoot(QWidget *parent);
                        ~FormBoot();

private slots:
    void                timerInterrupt();
    void on_buttonStart_clicked();
    void on_buttonWriteSettings_clicked();
    void on_buttonReadSettings_clicked();
    void on_buttonWriteCPU_clicked();
    void on_buttonWriteGUI_clicked();
    void on_buttonWriteManual_clicked();
    void on_buttonReadGUI_clicked();
    void on_buttonReadAudit_clicked();
    void on_btnWriteLang_clicked();
    void on_btnReadLang_clicked();


private:
    bool                priv_langCopy (const QString &srcFolder, const QString &dstFolder, long timeToWaitDuringCopyFinalizingMSec) const;
    void                foreverDisableBtnStartVMC();

private:
    Ui::FormBoot        *ui;
    QTimer              *timer;
    bool                bBtnStartVMCEnabled;

};

#endif // FORMBOOT_H
