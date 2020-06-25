#ifndef FORMBOOT_H
#define FORMBOOT_H
#include <QDialog>
#include <QLabel>
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

    void on_btnSkipAll_clicked();

    void on_btnSkipCPU_clicked();

    void on_btnSkipDA3_clicked();

    void on_btnSkipGUI_clicked();

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
        eUploadDA3CallBack_btn = 1,
        eUploadDA3CallBack_auto = 2
    };

    enum eUploadCPUFWCallBack
    {
        eUploadCPUFWCallBack_none = 0,
        eUploadCPUFWCallBack_btn = 1,
        eUploadCPUFWCallBack_auto = 2
    };

    enum eAutoUpdateFase
    {
        eAutoUpdateFase_begin,

        eAutoUpdateFase_cpu_start,
        eAutoUpdateFase_cpu_waitForSkip,
        eAutoUpdateFase_cpu_upload,
        eAutoUpdateFase_cpu_upload_wait,
        eAutoUpdateFase_cpu_upload_finishedOK,
        eAutoUpdateFase_cpu_upload_finishedKO,

        eAutoUpdateFase_gui_start,
        eAutoUpdateFase_gui_waitForSkip,
        eAutoUpdateFase_gui_upload,
        eAutoUpdateFase_gui_upload_copy,

        eAutoUpdateFase_da3_start,
        eAutoUpdateFase_da3_waitForSkip,
        eAutoUpdateFase_da3_upload,
        eAutoUpdateFase_da3_upload_wait,
        eAutoUpdateFase_da3_upload_finishedOK,
        eAutoUpdateFase_da3_upload_finishedKO,

        eAutoUpdateFase_finished,
        eAutoUpdateFase_finishedKO,
        eAutoUpdateFase_finished_wait,
        eAutoUpdateFase_backToFormBoot
    };

private:
    struct sAutoupdate
    {
        bool        isRunning;
        eAutoUpdateFase  fase;
        u8          skipInHowManySec;
        u64         nextTimeTickMSec;
        char        cpuFileName[256];
        char        da3FileName[256];
        char        guiFolderName[256];
    };

private:
    void                    priv_pleaseWaitShow (const char *message);
    void                    priv_pleaseWaitHide();
    void                    priv_pleaseSetTextWithColor (const char *message, const char *bgColor, const char *textColor);
    void                    priv_pleaseWaitSetText (const char *message);
    void                    priv_pleaseWaitSetError (const char *message);
    void                    priv_pleaseWaitSetOK (const char *message);

    void                    priv_fileListShow(eFileListMode mode);
    void                    priv_fileListPopulate(const u8 *pathNoSlash, const u8 *jolly, bool bClearList);
    void                    priv_fileListHide();

    void                    priv_updateLabelInfo();
    void                    priv_onCPUBridgeNotification (rhea::thread::sMsg &msg);
    void                    priv_onESAPINotification (rhea::thread::sMsg &msg);
    bool                    priv_langCopy (const u8 *srcFolder, const u8 *dstFolder, u32 timeToWaitDuringCopyFinalizingMSec);
    void                    priv_foreverDisableBtnStartVMC();
    void                    priv_on_btnInstall_DA3_onFileSelected (const u8 *srcFullFilePathAndName);
    void                    priv_uploadManual (const u8 *srcFullFilePathAndName);
    void                    priv_uploadGUI (const u8 *srcFullFolderPath);
    void                    priv_on_btnInstall_CPU_onFileSelected (const u8 *fullFilePathAndName);
    void                    priv_syncUSBFileSystem(u64 minTimeMSecToWaitMSec);
    void                    priv_enableButton (QPushButton *btn, bool bEnabled);

    void                    priv_startDownloadDataAudit (eDwnloadDataAuditCallBack mode);
    void                    priv_startUploadDA3 (eUploadDA3CallBack mode, const u8 *fullFilePathAndName);
    void                    priv_startUploadCPUFW (eUploadCPUFWCallBack mode, const u8 *fullFilePathAndName);
    void                    priv_on_btnDownload_audit_download (rhea::thread::sMsg &msg);
    void                    priv_on_btnDownload_diagnostic_downloadDataAudit (rhea::thread::sMsg &msg);
    void                    priv_on_btnDownload_diagnostic_makeZip();
    void                    priv_on_btnInstall_DA3_upload (rhea::thread::sMsg &msg);
    void                    priv_on_btnInstall_CPU_upload (rhea::thread::sMsg &msg);
    bool                    priv_doInstallGUI (const u8 *srcFullFolderPath) const;
    void                    priv_uploadESAPI_GUI (rhea::thread::sMsg &msg);
    void                    priv_uploadESAPI_GUI_unzipped(rhea::thread::sMsg &msg);

    bool                    priv_autoupdate_exists();
    void                    priv_autoupdate_showForm();
    void                    priv_autoupdate_onTick();
    void                    priv_autoupdate_setTextWithColor (QLabel *lab, const char *message, const char *bgColor, const char *textColor);
    void                    priv_autoupdate_setText (QLabel *lab, const char *message);
    void                    priv_autoupdate_setOK (QLabel *lab, const char *message);
    void                    priv_autoupdate_setError (QLabel *lab, const char *message);
    void                    priv_autoupdate_center (QLabel *lab);
    void                    priv_autoupdate_toTheLeft (QLabel *lab);
    void                    priv_autoupdate_onCPU_upload (rhea::thread::sMsg &msg);
    void                    priv_autoupdate_onDA3_upload (rhea::thread::sMsg &msg);

private:
    sGlobal                 *glob;
    Ui::FormBoot            *ui;
    bool                    bBtnStartVMCEnabled;
    QChar                   msgCPU[128];
    eFileListMode           fileListShowMode;
    eRetCode                retCode;
    eDwnloadDataAuditCallBack dwnloadDataAuditCallBack;
    eUploadDA3CallBack      upldDA3CallBack;
    eUploadCPUFWCallBack    upldCPUFWCallBack;
    sAutoupdate             autoupdate;
    u32                     sizeInBytesOfCurrentFileUnpload;

};

#endif // FORMBOOT_H
