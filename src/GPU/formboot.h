#ifndef FORMBOOT_H
#define FORMBOOT_H
#include <QDialog>
#include "header.h"
#include "../CPUBridge/CPUBridge.h"

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
    explicit            FormBoot(QWidget *parent, sGlobal *glob);
                        ~FormBoot();

    void                showMe();
    int                 onTick();

private slots:
    void                on_buttonStart_clicked();
    void                on_btnInstall_languages_clicked();
    void                on_btnInstall_manual_clicked();
    void                on_btnDownload_audit_clicked();
    void                on_btnInstall_DA3_clicked();
    void                on_btnOK_clicked();
    void                on_btnCancel_clicked();
    void                on_lbFileList_doubleClicked(const QModelIndex &index);

    void on_btnDownload_DA3_clicked();

    void on_btnInstall_GUI_clicked();

    void on_btnDownload_GUI_clicked();

    void on_btnDownload_diagnostic_clicked();

    void on_btnInstall_CPU_clicked();

private:
    enum eFileListMode
    {
        eFileListMode_CPU = 0,
        eFileListMode_DA3,
        eFileListMode_GUI,
        eFileListMode_Manual
    };

private:
    void                    priv_setButtonStyle (QPushButton *obj, const char *style);
    void                    priv_pleaseWaitShow (const char *message);
    void                    priv_pleaseWaitHide();
    void                    priv_pleaseWaitSetText (const char *message);
    void                    priv_pleaseWaitSetError (const char *message);

    void                    priv_fileListShow(eFileListMode mode);
    void                    priv_fileListPopulate(const char *pathNoSlash, const char *jolly, bool bClearList);
    void                    priv_fileListHide();

    void                    priv_updateLabelInfo();
    void                    priv_onCPUBridgeNotification (rhea::thread::sMsg &msg);
    bool                    priv_langCopy (const char *srcFolder, const char *dstFolder, u32 timeToWaitDuringCopyFinalizingMSec);
    void                    foreverDisableBtnStartVMC();
    void                    priv_uploadDA3 (const char *srcFullFilePathAndName);
    void                    priv_uploadManual (const char *srcFullFilePathAndName);
    void                    priv_uploadGUI (const char *srcFullFolderPath);
    void                    priv_uploadCPUFW (const char *fullFilePathAndName);


private:
    sGlobal                 *glob;
    Ui::FormBoot            *ui;
    bool                    bBtnStartVMCEnabled;
    QChar                   msgCPU[128];
    eFileListMode           fileListShowMode;
    int                     retCode;
    QFont                   fntButton;
    QFont                   fnt12;

};

#endif // FORMBOOT_H
