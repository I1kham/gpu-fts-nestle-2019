#include "header.h"
#include "formprog.h"
#include "ui_formprog.h"
#include "Utils.h"
#include <QWidget>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QWebView>
#include <QWebFrame>
#include <QUrl>



bool isManualPresent();

extern unsigned char button_keyNum_tx;
extern QString CPU_version;
extern QString Folder_Manual;


FormProg::FormProg(QWidget *parent) :    QDialog(parent),    ui(new Ui::FormProg)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    ui->debugTextBox->setVisible(false);
}

FormProg::~FormProg()
{
    delete ui;
}



void FormProg::initForm(const char *iso2LettersLanguageCode)
{
 //   QApplication::setOverrideCursor(Qt::PointingHandCursor);

    updateLabelVersion();
    ui->labelVersion->raise();
    ui->labelVersion->setAlignment(Qt::AlignRight);

    
    if(isManualPresent()==false)    
    {
        ui->buttonManualOpen->hide();   
    }
    ui->buttonManualQuit->hide();

    ui->labelStatus->setGeometry((ScreenW-labelStatus_Prog_W)/2 ,ScreenH-labelStatusProg_H-labelStatus_MarginBottom-30, labelStatus_Prog_W, labelStatusProg_H);
    //ui->labelStatus->setStyleSheet("QLabel { background-color: #808080; color:#fff; }");
    ui->labelStatus->setText("");
    ui->labelStatus->raise();
    ui->labelStatus->setAlignment(Qt::AlignCenter);

    utils::getRightFontForLanguage (theFont, 20, iso2LettersLanguageCode);
    ui->labelStatus->setFont (theFont);

    utils::getRightFontForLanguage (theFontSmall, 10, iso2LettersLanguageCode);
    ui->debugTextBox->setFont(theFontSmall);
}



void FormProg::updateLabelStatusProg(QString qs_p)
{
    ui->labelStatus->setText(qs_p);
}

void FormProg::updateLabelVersion()
{
    char str[32];
    sprintf (str, "%d.%d.%d", GPU_VERSION_MAJOR, GPU_VERSION_MINOR, GPU_VERSION_BUILD);
    ui->labelVersion->setText("GPU version: " + QString(str) + "  -  CPU:" + CPU_version);
}


void FormProg::on_buttonB1_pressed()
{
    
    setButtonKeyNum(1);
}
void FormProg::on_buttonB2_pressed()
{
    
    setButtonKeyNum(2);
}
void FormProg::on_buttonB3_pressed()
{
    
    setButtonKeyNum(3);
}
void FormProg::on_buttonB4_pressed()
{
    
    setButtonKeyNum(4);
}
void FormProg::on_buttonB5_pressed()
{
    
    setButtonKeyNum(5);
}
void FormProg::on_buttonB6_pressed()
{
    
    setButtonKeyNum(6);
}
void FormProg::on_buttonB7_pressed()
{
    
    setButtonKeyNum(7);
}
void FormProg::on_buttonB8_pressed()
{
    
    setButtonKeyNum(8);
}
void FormProg::on_buttonB9_pressed()
{
    
    setButtonKeyNum(9);
}
void FormProg::on_buttonB10_pressed()
{
    
    setButtonKeyNum(10);
}





void FormProg::on_buttonB1_released()
{
    setButtonKeyNum(0);
}
void FormProg::on_buttonB2_released()
{
    setButtonKeyNum(0);
}
void FormProg::on_buttonB3_released()
{
    setButtonKeyNum(0);
}
void FormProg::on_buttonB4_released()
{
    setButtonKeyNum(0);
}
void FormProg::on_buttonB5_released()
{
    setButtonKeyNum(0);
}
void FormProg::on_buttonB6_released()
{
    setButtonKeyNum(0);
}
void FormProg::on_buttonB7_released()
{
    setButtonKeyNum(0);
}
void FormProg::on_buttonB8_released()
{
    setButtonKeyNum(0);
}
void FormProg::on_buttonB9_released()
{
    setButtonKeyNum(0);
}
void FormProg::on_buttonB10_released()
{
    setButtonKeyNum(0);
}




QWebView * m_pWebView;
const int webManual_sideBar_w=84;
const int webManual_W=ScreenW-webManual_sideBar_w;  
QString sHtmlUrlManual;

bool isManualPresent()
{
    sHtmlUrlManual="";
    QDir directoryManualRoot(Folder_Manual);
    QStringList ManualFilesAndDirectories = directoryManualRoot.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Time);
    if(ManualFilesAndDirectories.count()==0)    
    {
        
        return false;
    }

    QString ManualRoot = Folder_Manual + "/" + ManualFilesAndDirectories[0];    
    QDir directoryManualRoot_B(ManualRoot);
    QFileInfoList ManualFilesAndDirectories_B = directoryManualRoot_B.entryInfoList();
    if(ManualFilesAndDirectories_B.count()<=3)    
    {
        
        return false;
    }
    

    sHtmlUrlManual = "file://" + ManualRoot + "/index.html";

    return true;
}


void FormProg::on_buttonManualOpen_clicked()
{
    
    m_pWebView = new QWebView(this);
    m_pWebView->setGeometry(0,0,webManual_W,ScreenH);   
    m_pWebView->setMouseTracking(false); 
    m_pWebView->raise();   
    m_pWebView->show();
    m_pWebView->settings()->setAttribute(QWebSettings::PluginsEnabled,true);
    m_pWebView->settings()->setAttribute(QWebSettings::AutoLoadImages, true);
    m_pWebView->settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);
    m_pWebView->page()->mainFrame()->setScrollBarPolicy( Qt::Vertical, Qt::ScrollBarAsNeeded );
    

    m_pWebView->load(QUrl(sHtmlUrlManual));

    ui->labelVersion->hide();
    ui->buttonManualQuit->show();
}

/*
void on_m_pWebView_clicked()
{
    QMessageBox::information(NULL, "warning", "CLICK");
    m_pWebView->close();
}
*/

void FormProg::on_buttonManualQuit_clicked()
{
    ui->buttonManualQuit->hide();
    ui->labelVersion->show();

    m_pWebView->close();
    delete m_pWebView;
}

//*******************************************************************
void FormProg::addDebugString(const char *text)
{
    if (text == NULL)
        return;
    if (text[0] == 0x00)
        return;
    addDebugString(QString(text));
}

//*******************************************************************
void FormProg::addDebugString(const QString &text)
{

    ui->debugTextBox->setVisible(true);
    QString s = ui->debugTextBox->toPlainText();
    if (s.length() > 12000)
    {
        s = s.mid (6000);
        ui->debugTextBox->moveCursor (QTextCursor::Start);
        ui->debugTextBox->clear();
        ui->debugTextBox->insertPlainText (s);
        ui->debugTextBox->moveCursor (QTextCursor::End);
    }

    long timeNowMSec = getTimeNowMSec();
    long sec  = timeNowMSec / 1000;
    long msec = timeNowMSec - sec*1000;
    char tt[16];
    sprintf (tt, "%05ld.%03ld ", sec, msec);

    ui->debugTextBox->moveCursor (QTextCursor::End);
    ui->debugTextBox->insertPlainText (tt);
    ui->debugTextBox->insertPlainText (text);
    ui->debugTextBox->insertPlainText ("\r\n");
    ui->debugTextBox->moveCursor (QTextCursor::End);
}
