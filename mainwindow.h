#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTimer>
#include <QtCharts>

#include "calibclient.h"
#include "uploadclient.h"
#include "databasemng.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connServerBtn_clicked();
    void on_connDevBtn_clicked();

    void onServerConnected();
    void onServerDisconnected();
    void onServerError(QAbstractSocket::SocketError error);

    void onDevConnected();
    void onDevDisconnected();
    void onDevConnError(QAbstractSocket::SocketError error);
    void on_startUploadBtn_clicked();

    void onCpcData(QString, QMap<QString,QString>);
    void on_testBtn_clicked();
    void onSecTimerTimeout();

    void on_preMonBtn_clicked();

    void on_nextMonBtn_clicked();

    void on_exportRecordBtn_clicked();

private:
    QTimer *secTimer;

    Ui::MainWindow *ui;
    QSettings *setting;
    UploadClient *upClient;
    CalibClient *cpcClient;
    AQIDataBaseMng *mng;

private:
    void initChartsView();
    void loadSettings();
    void loadHistoyChartView1(int dir);

    QChart *liveChart;
    QChartView *liveChartView;
    QValueAxis *xAxis, *yAxis;
    QLineSeries *a18LineSerial, *a19LineSerial;

    QChart *historyChart;
    QChartView *historyChartView;
    QValueAxis *xAxisH, *yAxisH;
    QLineSeries *a18LineSerialH, *a19LineSerialH;


    int secDataCnt = 0;
    int hourDataCnt = 0;
    double minA18Sum = 0;
    double minA19Sum = 0;
    double hourA19Sum = 0;
    double hourA18Sum = 0;
};
#endif // MAINWINDOW_H
