#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    cpcClient = new CalibClient();
    connect(cpcClient, SIGNAL(sigConnected()), this, SLOT(onDevConnected()));
    connect(cpcClient, SIGNAL(sigDisConnected()), this, SLOT(onDevDisconnected()));
    connect(cpcClient, SIGNAL(sigReadData(QString,QMap<QString,QString>)),
            this, SLOT(onCpcData(QString,QMap<QString,QString>)));

    upClient =  new UploadClient();
    connect(upClient, SIGNAL(connected()), this, SLOT(onServerConnected()));
    connect(upClient, SIGNAL(disconnected()), this, SLOT(onServerDisconnected()));
    connect(upClient, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onServerError(QAbstractSocket::SocketError)));

    loadSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadSettings()
{
    setting = new QSettings("setting.ini", QSettings::IniFormat);
    if(setting->contains("deviceIp"))
        ui->devIp->setText(setting->value("deviceIp").toString());
    if(setting->contains("devicePort"))
        ui->devPort->setText(setting->value("devicePort").toString());
    if(setting->contains("serverIp"))
        ui->serverIp->setText(setting->value("serverIp").toString());
    if(setting->contains("serverPort"))
        ui->serverPort->setText(setting->value("serverPort").toString());
}

void MainWindow::onDevConnected()
{
    qDebug()<<"server connected";
    ui->devIp->setStyleSheet("background-color: rgb(0, 255, 127);");
    ui->connDevBtn->setText("断开连接");
    setting->setValue("deviceIp", ui->devIp->text());
    setting->setValue("devicePort", ui->devPort->text());
}

void MainWindow::onDevDisconnected()
{
    qDebug()<<"server disconnected";
    ui->devIp->setStyleSheet("background-color: rgb(255, 255, 127);");
    ui->connDevBtn->setText("连接");
}

void MainWindow::onServerConnected()
{
    qDebug()<<"server connected";
    ui->serverIp->setStyleSheet("background-color: rgb(0, 255, 127);");
    ui->connServerBtn->setText("断开连接");

    setting->setValue("serverIp", ui->serverIp->text());
    setting->setValue("serverPort", ui->serverPort->text());
}

void MainWindow::onServerDisconnected()
{
    qDebug()<<"server disconnected";
    ui->serverIp->setStyleSheet("background-color: rgb(255, 255, 127);");
    ui->connServerBtn->setText("连接");
}

void MainWindow::onServerError(QAbstractSocket::SocketError error)
{
    qDebug()<<"server error";
}
void MainWindow::on_connServerBtn_clicked()
{
    if(upClient->state() == QAbstractSocket::ConnectedState)
        upClient->disconnectFromHost();
    else if(upClient->state() != QAbstractSocket::ConnectingState)
        upClient->connectToHost(ui->serverIp->text(), ui->serverPort->text().toUInt());
}

void MainWindow::on_connDevBtn_clicked()
{
    if(cpcClient->state() == QAbstractSocket::ConnectedState)
        cpcClient->disconnectFromHost();
    else if(cpcClient->state() != QAbstractSocket::ConnectingState)
        cpcClient->connectToHost(ui->devIp->text(), ui->devPort->text().toUInt());
}

void MainWindow::onCpcData(QString client, QMap<QString, QString> data)
{

}

void MainWindow::on_startUploadBtn_clicked()
{
}
