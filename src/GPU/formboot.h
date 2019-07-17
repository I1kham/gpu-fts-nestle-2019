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

private:
    bool    bBtnStartVMCEnabled;

private:
    int     instalFonts(const QString  *srcFolder);
    bool    priv_langCopy (const QString &srcFolder, const QString &dstFolder) const;
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

public:
    Ui::FormBoot *ui;
};

#endif // FORMBOOT_H
