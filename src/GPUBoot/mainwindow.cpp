#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScreen>
#include <qdir.h>
#include <qprocess.h>
#include "main.h"


//**********************************************************************
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    const int height = screenGeometry.height();
    const int width = screenGeometry.width();

    ui->setupUi(this);
    ui->centralWidget->setFixedSize (width, height);
    ui->logPanel->setVisible(false);
    ui->btnCloseErrorLog->setVisible(false);
    ui->labError->setVisible(false);

    //versione
    ui->labVersion->setText ("SMU Bootloader v" VERSION);

    //ridimensionamento degli elementi grafici
    {
        const int BORDER_X = 10;
        const int BOTTOM_BUTTON_Y = height - ui->btnStartVMC->geometry().height() - BORDER_X;
        const int LAB_ERROR_Y = BOTTOM_BUTTON_Y - 60;
        int x,y,w,h;

        x = BORDER_X;
        y = ui->lbFileList->geometry().y();
        w = width - x - BORDER_X;
        h = LAB_ERROR_Y - y - BORDER_X;
        ui->lbFileList->setGeometry(x,y,w,h);

        x = BORDER_X;
        y = ui->labIntro->geometry().y();
        h = LAB_ERROR_Y - y -BORDER_X;
        ui->logPanel->setGeometry(x,y,w,h);

        x = BORDER_X;
        y = LAB_ERROR_Y;
        h = ui->labError->geometry().height();
        ui->labError->setGeometry(x,y,w,h);


        x = BORDER_X;
        y = BOTTOM_BUTTON_Y;
        ui->btnUpdateSMU->move (x,y);

        x = width - BORDER_X - ui->btnStartVMC->geometry().width();
        ui->btnStartVMC->move (x,y);
        ui->btnCloseErrorLog->move (x,y);
    }
    priv_fileListPopulate ();
}

//**********************************************************************
MainWindow::~MainWindow()
{
    delete ui;
}


//**********************************************************************
void MainWindow::priv_fileListPopulate ()
{
    hideMouse();
    ui->lbFileList->clear();
    QDir folder(RHEA_USB_FOLDER);

    //non dovrebbe mai accadere visto che controlliamo nel main..
    if (!folder.exists())
        return;

    QStringList list = folder.entryList(QStringList() << "*.mh210" << "*.mh210",QDir::Files);
    foreach (QString fname, list)
    {
        ui->lbFileList->addItem(fname);
    }

    if (ui->lbFileList->count())
        ui->lbFileList->item(0)->setSelected(true);
}

//**********************************************************************
void MainWindow::on_lbFileList_doubleClicked(const QModelIndex &index UNUSED_PARAM) { on_btnUpdateSMU_clicked(); }
void MainWindow::on_btnCloseErrorLog_clicked()                                      { priv_on_unpackMH210_finished(); }


//**********************************************************************
void MainWindow::on_btnStartVMC_clicked()
{
    QApplication::quit();
}

//**********************************************************************
void MainWindow::priv_setMessageOKWithColor (const QString &msg, const char *bgColor, const char *textColor)
{
    if (msg.length() == 0)
        ui->labError->setVisible(false);
    else
    {
        char s[96];
        sprintf (s, "QLabel { background-color:%s; color:%s; }", bgColor, textColor);
        ui->labError->setStyleSheet(s);
        ui->labError->setVisible(true);
        ui->labError->setText(msg);
    }
    QApplication::processEvents();
}

//**********************************************************************
void MainWindow::priv_setMessageOK (const QString &msg)
{
    priv_setMessageOKWithColor (msg, "#43b441", "#fff");
}

//**********************************************************************
void MainWindow::priv_setMessageKO (const QString &msg)
{
    priv_setMessageOKWithColor (msg, "#f00", "#fff");
}

//**********************************************************************
void MainWindow::on_btnUpdateSMU_clicked()
{
    ui->labError->setVisible(false);
    if (ui->lbFileList->selectedItems().count() == 0)
        return;

    QListWidgetItem *item = ui->lbFileList->selectedItems().at(0);
    QString srcFilename = item->text();

    ui->btnStartVMC->setVisible(false);
    ui->btnUpdateSMU->setVisible(false);
    ui->lbFileList->setVisible(false);

    ui->logPanel->clear();
    ui->logPanel->setVisible(true);
    ui->logPanel->raise();

    QString fullFilePathAndName = RHEA_USB_FOLDER;
    fullFilePathAndName += "/";
    fullFilePathAndName += srcFilename;

    if (!priv_unpackMH210 (fullFilePathAndName))
        ui->btnCloseErrorLog->setVisible(true);
    else
        priv_on_unpackMH210_finished();
}

//**********************************************************************
void MainWindow::priv_on_unpackMH210_finished()
{
    ui->btnCloseErrorLog->setVisible(false);
    ui->logPanel->setVisible(false);
    ui->btnStartVMC->setVisible(true);
    ui->btnUpdateSMU->setVisible(true);
    ui->lbFileList->setVisible(true);
}

//**********************************************************************
void MainWindow::priv_log (const QString &text)
{
    ui->logPanel->appendPlainText(text);
    ui->logPanel->verticalScrollBar()->setValue(ui->logPanel->verticalScrollBar()->maximum()); // Scrolls to the bottom
    QApplication::processEvents();
}

//**********************************************************************
bool MainWindow::priv_unpackMH210 (const QString &fullFilePathAndName)
{
    const char PACK_FILENAME[] = {"gpu_temp"};
    QString s;

    //copio il file src (che è un tar) nel folder del bootloader
    s = APPLICATION_FOLDER;
    s+= "/";
    s+=PACK_FILENAME;
    priv_log ("Copying " +fullFilePathAndName +" in " +s);
    if (QFile::exists(s))
            QFile::remove(s);
    if (!QFile::copy (fullFilePathAndName, s))
    {
        priv_log ("ERROR, unable to copy file");
        priv_setMessageKO("ERROR, unable to copy file");
        return false;
    }
    priv_log ("OK");


    //untar: tar -xvf "file"
    priv_log ("Decompressing file, please wait...");
    QProcess *proc = new QProcess();
    proc->setWorkingDirectory(APPLICATION_FOLDER);
    proc->start("tar", QStringList() << "-xvf" << PACK_FILENAME);
    proc->waitForFinished();
    int err = proc->exitCode();
    if (0 != err)
    {
        priv_log (QString("exit code %1").arg(err));
        priv_setMessageKO (QString("ERROR, exit code %1").arg(err));
        return false;
    }

    priv_log ("OK");
    priv_setMessageOK ("SMU was successfully updated");
    return true;
}

