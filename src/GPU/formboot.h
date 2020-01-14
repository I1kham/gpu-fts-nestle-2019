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
    eRetCode            onTick();

private slots:
    void                on_buttonStart_clicked();
    void                on_btnInstall_languages_clicked();
    void                on_btnInstall_manual_clicked();
    void                on_btnDownload_audit_clicked();
    void                on_btnInstall_DA3_clicked();
    void                on_btnOK_clicked();
    void                on_btnCancel_clicked();
    void                on_lbFileList_doubleClicked(const QModelIndex &index);
    void                on_btnDownload_DA3_clicked();
    void                on_btnInstall_GUI_clicked();
    void                on_btnDownload_GUI_clicked();
    void                on_btnDownload_diagnostic_clicked();
    void                on_btnInstall_CPU_clicked();

private:
    enum eFileListMode
    {
        eFileListMode_CPU = 0,
        eFileListMode_DA3,
        eFileListMode_GUI,
        eFileListMode_Manual
    };

    enum eDwnloadDataAuditCallBack
    {
        eDwnloadDataAuditCallBack_none = 0,
        eDwnloadDataAuditCallBack_btn = 1,
        eDwnloadDataAuditCallBack_service = 2
    };

    enum eUploadDA3CallBack
    {
        eUploadDA3CallBack_none = 0,
        eUploadDA3CallBack_btn = 1
    };

private:
    void                    priv_pleaseWaitShow (const char *message);
    void                    priv_pleaseWaitHide();
    void                    priv_pleaseSetTextWithColor (const char *message, const char *bgColor, const char *textColor);
    void                    priv_pleaseWaitSetText (const char *message);
    void                    priv_pleaseWaitSetError (const char *message);
    void                    priv_pleaseWaitSetOK (const char *message);

    void                    priv_fileListShow(eFileListMode mode);
    void                    priv_fileListPopulate(const char *pathNoSlash, const char *jolly, bool bClearList);
    void                    priv_fileListHide();

    void                    priv_updateLabelInfo();
    void                    priv_onCPUBridgeNotification (rhea::thread::sMsg &msg);
    bool                    priv_langCopy (const char *srcFolder, const char *dstFolder, u32 timeToWaitDuringCopyFinalizingMSec);
    void                    priv_foreverDisableBtnStartVMC();
    void                    priv_on_btnInstall_DA3_onFileSelected (const char *srcFullFilePathAndName);
    void                    priv_uploadManual (const char *srcFullFilePathAndName);
    void                    priv_uploadGUI (const char *srcFullFolderPath);
    void                    priv_uploadCPUFW (const char *fullFilePathAndName);
    void                    priv_syncUSBFileSystem(u64 minTimeMSecToWaitMSec);
    void                    priv_enableButton (QPushButton *btn, bool bEnabled);

    void                    priv_startDownloadDataAudit (eDwnloadDataAuditCallBack mode);
    void                    priv_startUploadDA3 (eUploadDA3CallBack mode, const char *fullFilePathAndName);
    void                    priv_on_btnDownload_audit_download (rhea::thread::sMsg &msg);
    void                    priv_on_btnDownload_diagnostic_downloadDataAudit (rhea::thread::sMsg &msg);
    void                    priv_on_btnDownload_diagnostic_makeZip();
    void                    priv_on_btnInstall_DA3_upload (rhea::thread::sMsg &msg);

private:
    sGlobal                 *glob;
    Ui::FormBoot            *ui;
    bool                    bBtnStartVMCEnabled;
    QChar                   msgCPU[128];
    eFileListMode           fileListShowMode;
    eRetCode                retCode;
    eDwnloadDataAuditCallBack dwnloadDataAuditCallBack;
    eUploadDA3CallBack      upldDA3CallBack;

};

#endif // FORMBOOT_H
