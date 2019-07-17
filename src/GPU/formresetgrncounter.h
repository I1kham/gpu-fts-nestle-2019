#ifndef FORMRESETGRNCOUNTER_H
#define FORMRESETGRNCOUNTER_H

#include <QWidget>

namespace Ui {
class FormResetGrnCounter;
}

class FormResetGrnCounter : public QWidget
{
    Q_OBJECT

public:
    explicit FormResetGrnCounter(QWidget *parent = 0);
    ~FormResetGrnCounter();

    void setButtonText (const char *text);
    int btnWasPressed;

private slots:
    void on_btnP1_clicked();

private:
    Ui::FormResetGrnCounter *ui;
};

#endif // FORMRESETGRNCOUNTER_H
