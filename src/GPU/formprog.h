#ifndef FORMPROG_H
#define FORMPROG_H

#include <QDialog>

namespace Ui {
class FormProg;
}

class FormProg : public QDialog
{
    Q_OBJECT

public:
    explicit FormProg(QWidget *parent = 0);
    ~FormProg();

   void initForm (const char *iso2LettersLanguageCode);
   void updateLabelStatusProg(QString qs_p);
   void updateLabelVersion();

   void addDebugString(const char *text);
   void addDebugString(const QString &text);

private slots:
    void on_buttonB1_pressed();

    void on_buttonB2_pressed();

    void on_buttonB3_pressed();

    void on_buttonB4_pressed();

    void on_buttonB5_pressed();

    void on_buttonB6_pressed();

    void on_buttonB7_pressed();

    void on_buttonB8_pressed();

    void on_buttonB9_pressed();

    void on_buttonB10_pressed();

    void on_buttonB1_released();

    void on_buttonB2_released();

    void on_buttonB3_released();

    void on_buttonB4_released();

    void on_buttonB5_released();

    void on_buttonB6_released();

    void on_buttonB7_released();

    void on_buttonB8_released();

    void on_buttonB9_released();

    void on_buttonB10_released();

    void on_buttonManualOpen_clicked();

    void on_buttonManualQuit_clicked();

private:
    Ui::FormProg *ui;
    QFont theFont,theFontSmall;
};

#endif // FORMPROG_H
