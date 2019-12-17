#ifndef FORMPREGUI_H
#define FORMPREGUI_H
#include "header.h"
#include <QDialog>
#include "../CPUBridge/CPUBridge.h"

namespace Ui
{
    class FormPreGui;
}

/*******************************************************************
 * FormPreGui
 *
 */
class FormPreGui : public QDialog
{
    Q_OBJECT

public:
    explicit                    FormPreGui(QWidget *parent, sGlobal *glob);
                                 ~FormPreGui();

    void                        showMe (u16 groundCounterLimit, bool bShowBtnCleaMilker);
    eRetCode                    onTick();

private:
    void                        priv_onCPUBridgeNotification (rhea::thread::sMsg &msg);
    void                        priv_setButtonStyle (QPushButton *btn, bool bEnabled);
    void                        priv_updateButtonClose();
    void                        priv_showMsg (const char *text);
    void                        priv_enableButtonResetGrndCounter (bool b);
    void                        priv_enableButtonCleanMilker (bool b);
    void                        priv_enableButtonClose (bool b);

private slots:
    void                        on_btnClose_clicked();
    void                        on_btnResetGrndCounter_clicked();

    void on_btnMilkModuleRInse_clicked();

private:
    Ui::FormPreGui              *ui;
    sGlobal                     *glob;
    QChar                       msgCPU[128];
    u64                         timeToAutoCloseMSec;
    eRetCode                    retCode;
    u16                         groundCounterLimit;
    bool                        bShowBtnCleanMilker;
};

#endif // FORMPREGUI_H
