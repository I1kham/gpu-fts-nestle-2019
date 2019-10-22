#include "header.h"
#include "formprog.h"
#include "ui_formprog.h"
#include "Utils.h"
#include <QWidget>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QUrl>


extern QString CPU_version;


//************************************************************
FormProg::FormProg(QWidget *parent, const sGlobal *globIN) :
        QDialog(parent),
        ui(new Ui::FormProg)
{
    glob = globIN;
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
}

//************************************************************
FormProg::~FormProg()
{
    delete ui;
}

//************************************************************
void FormProg::initForm(const char *iso2LettersLanguageCode)
{
 //   QApplication::setOverrideCursor(Qt::PointingHandCursor);

    updateLabelVersion();
    ui->labelVersion->raise();
    ui->labelVersion->setAlignment(Qt::AlignRight);

    //ui->labelStatus->setStyleSheet("QLabel { background-color: #808080; color:#fff; }");
    ui->labelStatus->setText("");
    utils::getRightFontForLanguage (theFont, 20, iso2LettersLanguageCode);
    ui->labelStatus->setFont (theFont);
    ui->labelStatus->raise();
}


//************************************************************
void FormProg::updateLabelStatusProg(QString qs_p)
{
    ui->labelStatus->setText(qs_p);
}

//************************************************************
void FormProg::updateLabelVersion()
{
    char str[32];
    sprintf (str, "%d.%d.%d", GPU_VERSION_MAJOR, GPU_VERSION_MINOR, GPU_VERSION_BUILD);
    ui->labelVersion->setText("GPU version: " + QString(str) + "  -  CPU:" + glob->cpuVersion);
}

//************************************************************
void FormProg::on_buttonB1_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 1); }
void FormProg::on_buttonB2_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 2); }
void FormProg::on_buttonB3_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 3); }
void FormProg::on_buttonB4_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 4); }
void FormProg::on_buttonB5_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 5); }
void FormProg::on_buttonB6_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 6); }
void FormProg::on_buttonB7_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 7); }
void FormProg::on_buttonB8_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 8); }
void FormProg::on_buttonB9_pressed()        { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 9); }
void FormProg::on_buttonB10_pressed()       { cpubridge::ask_CPU_SEND_BUTTON (glob->subscriber, 10); }

