#include "formresetgrncounter.h"
#include "ui_formresetgrncounter.h"

FormResetGrnCounter::FormResetGrnCounter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormResetGrnCounter)
{
    btnWasPressed = 0;
    ui->setupUi(this);
}

FormResetGrnCounter::~FormResetGrnCounter()
{
    delete ui;
}

void FormResetGrnCounter::on_btnP1_clicked()
{
    btnWasPressed = 1;
}

void FormResetGrnCounter::setButtonText (const char *text)
{
    ui->btnP1->setText (text);
}
