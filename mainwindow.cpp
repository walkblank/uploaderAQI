#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    cpcClient = new CalibClient();
    cpcClient->setClientType("cpc");
    connect(cpcClient, SIGNAL(sigConnected()), this, SLOT(onDevConnected()));
    connect(cpcClient, SIGNAL(sigDisConnected()), this, SLOT(onDevDisconnected()));
//    connect(cpcClient, SIGNAL(sigError(QAbstractSocket::SocketError)),
//            this, SLOT(onDevConnError(QAbstractSocket::SocketError)));
    connect(cpcClient, SIGNAL(sigReadData(QString,QMap<QString,QString>)),
            this, SLOT(onCpcData(QString,QMap<QString,QString>)));

    upClient =  new UploadClient();
    connect(upClient, SIGNAL(connected()), this, SLOT(onServerConnected()));
    connect(upClient, SIGNAL(disconnected()), this, SLOT(onServerDisconnected()));
    connect(upClient, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onServerError(QAbstractSocket::SocketError)));

    secTimer = new QTimer();
    connect(secTimer, SIGNAL(timeout()), this, SLOT(onSecTimerTimeout()));

    loadSettings();
    initChartsView();

    mng = new AQIDataBaseMng();
}

MainWindow::~MainWindow()
{
    delete ui;
}

static int a18ChartPointCnt = 0;
static int a19ChartPointCnt = 0;
static int curMaxPonitCnt = 60;

void MainWindow::onSecTimerTimeout()
{
    QDateTime curTime = QDateTime::currentDateTime();
    if(curTime.time().second() % 5 == 0)
        cpcClient->queryAQI();
    if(curTime.time().second() == 0)
    {
        QString timeStr = curTime.toString("yyyyddMMhhmmss");
        double averageA18Value = minA18Sum/secDataCnt;
        double averageA19Value = minA19Sum/secDataCnt;
        qDebug()<<"min average A18" << averageA18Value;
        qDebug()<<"min average A18" << averageA19Value;
        upClient->uploadMinData(timeStr, averageA18Value, averageA19Value);
        secDataCnt = 0;
        minA18Sum = 0;
        minA19Sum = 0;
        a18ChartPointCnt += 1;
        a19ChartPointCnt += 1;
        a18LineSerial->append(a18ChartPointCnt, averageA18Value);
        a19LineSerial->append(a19ChartPointCnt, averageA19Value);

        qDebug()<<"curMaxPointCnt"<<curMaxPonitCnt;
        qDebug()<<"a18PointCnt" << a18ChartPointCnt;
        if(a18ChartPointCnt > curMaxPonitCnt)
        {
            curMaxPonitCnt = curMaxPonitCnt*2;
            if(curMaxPonitCnt > 1440)
                curMaxPonitCnt = 1440;
            xAxis->setRange(0, curMaxPonitCnt);
        }

        if(curTime.time().minute() == 0)
        {
            double avHourA18Val = hourA18Sum/hourDataCnt;
            double avHourA19Val = hourA19Sum/hourDataCnt;
            upClient->uploadHourData(timeStr, avHourA18Val, avHourA19Val);
            hourA18Sum = 0;
            hourA19Sum = 0;
            hourDataCnt = 0;
        }
    }

    if(curTime.time().hour() == 0 && curTime.time().second() == 0 && curTime.time().minute() == 0)
    {
        curMaxPonitCnt = 60;
        a18ChartPointCnt = 0;
        a19ChartPointCnt = 0;
        xAxis->setRange(0, curMaxPonitCnt);
        a18LineSerial->clear();
        a19LineSerial->clear();
    }

}

void MainWindow::initChartsView()
{
    liveChart = new QChart();
    liveChartView = new QChartView(liveChart);
    liveChartView->setMinimumSize(1200, 600);
    a18LineSerial = new QLineSeries();
    a18LineSerial->setName("A18");
    a19LineSerial = new QLineSeries();
    a19LineSerial->setName("A19");

    xAxis = new QValueAxis();
    xAxis->setRange(0, 60);
    xAxis->setTickCount(31);
    xAxis->setMinorTickCount(0);
    xAxis->setLabelFormat("%d");
    xAxis->setTitleText("time(min)");

    yAxis = new QValueAxis();
    yAxis->setRange(0, 100);
    yAxis->setTickCount(11);
    yAxis->setMinorTickCount(1);
    yAxis->setLabelFormat("%.2f");
    yAxis->setTitleText("AQI");

    liveChart->addAxis(xAxis, Qt::AlignBottom);
    liveChart->addAxis(yAxis, Qt::AlignLeft);
    liveChart->addSeries(a18LineSerial);
    liveChart->addSeries(a19LineSerial);

    a18LineSerial->attachAxis(yAxis);
    a18LineSerial->attachAxis(xAxis);
    a19LineSerial->attachAxis(yAxis);
    a19LineSerial->attachAxis(xAxis);

    ui->liveCharView->addWidget(liveChartView);
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
    secTimer->stop();
    ui->startUploadBtn->setText("开始上传");
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
    secTimer->stop();
    ui->startUploadBtn->setText("开始上传");
}

void MainWindow::onServerError(QAbstractSocket::SocketError error)
{
    qDebug()<<"server error";
    ui->serverIp->setStyleSheet("background-color: rgb(255, 0, 0);");
    ui->connServerBtn->setText("连接");
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

void MainWindow::onDevConnError(QAbstractSocket::SocketError error)
{
    qDebug()<<"server error";
    ui->devIp->setStyleSheet("background-color: rgb(255, 0, 0);");
    ui->connDevBtn->setText("连接");
}

void MainWindow::onCpcData(QString client, QMap<QString, QString> data)
{
    qDebug()<<" on data " << client << data;
    if(data.keys().contains("62"))
    {
        tempA18Datas.append(data["62"].toDouble());
        tempA19Datas.append(data["64"].toDouble());
        minA18Sum += data["62"].toDouble();
        minA19Sum += data["64"].toDouble();
        hourA18Sum += data["62"].toDouble();
        hourA19Sum += data["64"].toDouble();
        secDataCnt += 1;
        hourDataCnt += 1;

        qDebug()<<"minA18Sum" << minA18Sum;
        qDebug()<<"minA19Sum" << minA19Sum;
    }
}

void MainWindow::on_startUploadBtn_clicked()
{
    if(secTimer->isActive())
    {
        ui->startUploadBtn->setText("开始上传");
        secTimer->stop();
    }
    else
    {
        a18LineSerial->clear();
        a19LineSerial->clear();
        curMaxPonitCnt = 60;
        a18ChartPointCnt = 0;
        a19ChartPointCnt = 0;
        xAxis->setRange(0, 60);
        ui->startUploadBtn->setText("停止上传");
        secTimer->start(1000);
    }
}

void MainWindow::on_testBtn_clicked()
{
    QSqlTableModel *model = mng->getRecordTable(QString());
    qDebug()<<model->rowCount() << model->record(0);
    static double value = 30;
    static int x = 1;
    if(x > curMaxPonitCnt)
    {
        curMaxPonitCnt = curMaxPonitCnt*2;
        if(curMaxPonitCnt > 1440)
            curMaxPonitCnt = 1440;
        xAxis->setRange(0, curMaxPonitCnt);
    }

    if(x == 120)
    {
        xAxis->setRange(0, 60);
        a18LineSerial->clear();
        x = 0;
        curMaxPonitCnt = 60;
    }

    a18LineSerial->append(x++, x%2 == 0 ? value++ : value--);
}
