#ifndef FORMPROG_H
#define FORMPROG_H
#include "header.h"
#include <QDialog>
#include "../CPUBridge/CPUBridge.h"

namespace Ui
{
    class FormProg;
}

/*******************************************************************
 * FormProg
 *
 */
class FormProg : public QDialog
{
    Q_OBJECT

public:
    explicit                    FormProg(QWidget *parent, const sGlobal *glob);
                                ~FormProg();

    void                        showMe();
    int                         onTick();

private slots:
    void                        on_buttonB1_pressed();
    void                        on_buttonB2_pressed();
    void                        on_buttonB3_pressed();
    void                        on_buttonB4_pressed();
    void                        on_buttonB5_pressed();
    void                        on_buttonB6_pressed();
    void                        on_buttonB7_pressed();
    void                        on_buttonB8_pressed();
    void                        on_buttonB9_pressed();
    void                        on_buttonB10_pressed();
    void                        on_buttonB1_released();
    void                        on_buttonB2_released();
    void                        on_buttonB3_released();
    void                        on_buttonB4_released();
    void                        on_buttonB5_released();
    void                        on_buttonB6_released();
    void                        on_buttonB7_released();
    void                        on_buttonB8_released();
    void                        on_buttonB9_released();
    void                        on_buttonB10_released();

private:
    void                        priv_onCPUBridgeNotification (rhea::thread::sMsg &msg);
    void                        priv_updateLabelVersion();

private:
    Ui::FormProg                *ui;
    const sGlobal               *glob;
    QFont                       theFont,theFontSmall;
    int                         retCode;
    QChar                       msgCPU[128];
};

#endif // FORMPROG_H
