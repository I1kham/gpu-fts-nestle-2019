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

   void                        initForm (const char *iso2LettersLanguageCode);
   void                        updateLabelStatusProg(QString qs_p);
   void                        updateLabelVersion();

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

private:
    Ui::FormProg                *ui;
    const sGlobal               *glob;
    QFont                       theFont,theFontSmall;
};

#endif // FORMPROG_H
