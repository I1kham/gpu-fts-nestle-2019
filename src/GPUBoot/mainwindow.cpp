#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScreen>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    const int height = screenGeometry.height();
    const int width = screenGeometry.width();

    ui->setupUi(this);
    ui->centralWidget->setFixedSize (width, height);
}

MainWindow::~MainWindow()
{
    delete ui;
}
