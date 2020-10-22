#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "calibclient.h"
#include "uploadclient.h"

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
//    void onDevError(QAbstractSocket::SocketError error);

    void on_startUploadBtn_clicked();

    void onCpcData(QString, QMap<QString,QString>);

private:
    Ui::MainWindow *ui;

    UploadClient *upClient;
    CalibClient *cpcClient;

private:
    void initChartsView();
};
#endif // MAINWINDOW_H
