#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QTime>

static int a18ChartPointCnt = 0;
static int a19ChartPointCnt = 0;
static int curMaxPonitCnt = 60;
static int currentIndex = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mng = new AQIDataBaseMng();
    cpcClient = new CalibClient();
    cpcClient->setClientType("cpc");
    connect(cpcClient, SIGNAL(sigConnected()), this, SLOT(onDevConnected()));
    connect(cpcClient, SIGNAL(sigDisConnected()), this, SLOT(onDevDisconnected()));
//    connect(cpcClient, SIGNAL(sigError(QAbstractSocket::SocketError)),
//            this, SLOT(onDevConnError(QAbstractSocket::SocketError)));
    connect(cpcClient, SIGNAL(sigReadData(QString,QMap<QString,QString>)),
            this, SLOT(onCpcData(QString,QMap<QString,QString>)));

    upClient =  new UploadClient(mng);
    connect(upClient, SIGNAL(connected()), this, SLOT(onServerConnected()));
    connect(upClient, SIGNAL(disconnected()), this, SLOT(onServerDisconnected()));
    connect(upClient, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onServerError(QAbstractSocket::SocketError)));

    secTimer = new QTimer();
    connect(secTimer, SIGNAL(timeout()), this, SLOT(onSecTimerTimeout()));

    loadSettings();
    initChartsView();
    loadHistoyChartView1(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onSecTimerTimeout()
{
    QDateTime curTime = QDateTime::currentDateTime();
    if(curTime.time().second() % 5 == 0)
        cpcClient->queryAQI();
    if(curTime.time().second() == 0)
    {
        QString timeStr = curTime.toString("yyyyMMddhhmmss");
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
        if(a18ChartPointCnt == 1)
            ui->liveStartTime->setText(curTime.toString("yyyy/MM/dd-hh:mm:ss"));
        a18LineSerial->append(a18ChartPointCnt, averageA18Value);
        a19LineSerial->append(a19ChartPointCnt, averageA19Value);
        ui->liveEndTime->setText(curTime.toString("yyyy/MM/dd-hh:mm:ss"));

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
            mng->addRecord(timeStr, QString("%1").arg(avHourA18Val), QString("%1").arg(avHourA19Val));
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

// dir 0 forward, 1  backward
void MainWindow::loadHistoyChartView1(int direction)
{
    static QList<int> startIndexes;
    static int curPos = 0;

    mng->setCurrFilter(QString());
    QSqlTableModel *tempModel = mng->getTableModel();
    while(tempModel->canFetchMore())
        tempModel->fetchMore();
    int recordCnt = tempModel->rowCount();

    if(direction == 0)
    {
        if(currentIndex >= recordCnt)
            return;
        curPos += 1;
    }
    else if(direction == 1)
    {
        curPos -= 1;
        currentIndex = startIndexes[curPos];
    }

    if(curPos == -1)
    {
        curPos = 0;
        return;
    }


    if(curPos > startIndexes.size())
        startIndexes.append(currentIndex);




    a18LineSerialH->clear();
    a19LineSerialH->clear();

    QSqlRecord startRecord = tempModel->record(currentIndex);

    QDateTime startRecordTime = QDateTime::fromString(startRecord.value("timestamp").toString(),
                                                      "yyyyMMddhhmmss");
    qDebug()<<"startRecordTime" << startRecordTime;
    QDateTime startTime = startRecordTime.addSecs(-3600*24*(startRecordTime.date().day()-1)-3600*startRecordTime.time().hour());
    qDebug()<<"startTime" << startTime << startTime.date().daysInMonth();
    ui->startTime->setText(startTime.toString("yyyy/MM/dd hh:mm:ss"));

    for(int i = 0; i < 744; i ++)
    {
        QDateTime tempTime;
        tempTime = startTime.addSecs(3600*i);
        if(tempTime.date().month() != startTime.date().month())
            break;
        qDebug()<<"tempTime" << tempTime.toString("yyyyMMddhhmmss");
        if(tempTime.toString("yyyyMMddhhmmss") == tempModel->record(currentIndex).value("timestamp").toString())
        {
            a18LineSerialH->append(i,tempModel->record(currentIndex).value("a18").toString().toDouble());
            a19LineSerialH->append(i,tempModel->record(currentIndex).value("a19").toString().toDouble());
            currentIndex += 1;
        }
        else
        {
            a18LineSerialH->append(i,0);
            a19LineSerialH->append(i,0);
        }

        ui->endTime->setText(tempTime.toString("yyyy/MM/dd hh:mm:ss"));
    }
}

void MainWindow::initChartsView()
{
    liveChart = new QChart();
    liveChartView = new QChartView(liveChart);
    liveChartView->setMinimumSize(1000, 600);
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

    historyChart= new QChart();
    historyChartView = new QChartView(historyChart);
    liveChartView->setMinimumSize(1000, 600);
    a18LineSerialH = new QLineSeries();
    a18LineSerialH->setName("A18");
    a19LineSerialH = new QLineSeries();
    a19LineSerialH->setName("A19");

    xAxisH = new QValueAxis();
    xAxisH->setRange(0, 744);
    xAxisH->setTickCount(32);
    xAxisH->setMinorTickCount(2);
    xAxisH->setLabelFormat("%d");
    xAxisH->setTitleText("time(Hour)");

    yAxisH = new QValueAxis();
    yAxisH->setRange(0, 100);
    yAxisH->setTickCount(11);
    yAxisH->setMinorTickCount(1);
    yAxisH->setLabelFormat("%.2f");
    yAxisH->setTitleText("AQI");

    historyChart->addAxis(xAxisH, Qt::AlignBottom);
    historyChart->addAxis(yAxisH, Qt::AlignLeft);
    historyChart->addSeries(a18LineSerialH);
    historyChart->addSeries(a19LineSerialH);

    a18LineSerialH->attachAxis(yAxisH);
    a18LineSerialH->attachAxis(xAxisH);
    a19LineSerialH->attachAxis(yAxisH);
    a19LineSerialH->attachAxis(xAxisH);

    ui->historyChartView->addWidget(historyChartView);

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
        minA18Sum += data["62"].toDouble();
        minA19Sum += data["64"].toDouble();
        hourA18Sum += data["62"].toDouble();
        hourA19Sum += data["64"].toDouble();
        secDataCnt += 1;
        hourDataCnt += 1;

        qDebug()<<"minA18Sum" << minA18Sum << "hourA18Sum" << QString::number(hourA18Sum, 'f', 2)  << hourA18Sum;
        qDebug()<<"minA19Sum" << minA19Sum << "hourA19Sum" << QString::number(hourA19Sum, 'f', 2)  << hourA19Sum;
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
        if(cpcClient->state() != QAbstractSocket::ConnectedState
                || upClient->state() != QAbstractSocket::ConnectedState)
        {
            QMessageBox::warning(this, "提醒", "服务器或设备未连接", QMessageBox::Ok);
            return;
        }
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
    static double value = 30;
    QDateTime tmpTime = QDateTime::fromString("20201011000000", "yyyyMMddhhmmss");
    for(int i = 0; i < 8192; i ++)
    {
        qDebug()<<"time+" << tmpTime.addSecs(i*3600);
        mng->addRecord(tmpTime.addSecs(i*3600).toString("yyyyMMddhhmmss"), QString("%1").arg(i%2 == 0 ? 30 : 35),
                       QString("%1").arg(i%2 == 0 ? 40 : 50));
    }
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

void MainWindow::on_preMonBtn_clicked()
{

   loadHistoyChartView1(1);
}

void MainWindow::on_nextMonBtn_clicked()
{
   loadHistoyChartView1(0);
}

void MainWindow::on_exportRecordBtn_clicked()
{

}
