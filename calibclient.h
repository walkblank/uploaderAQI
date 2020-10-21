#ifndef CALIBCLIENT_H
#define CALIBCLIENT_H

#include <QObject>
#include <QRandomGenerator>
#include <QTcpSocket>

class CalibClient : public QObject
{
    Q_OBJECT
public:
    CalibClient();
    CalibClient(QString s);
    void AttachSocket(QTcpSocket *sock);

    void setClientType(QString type) {clientType = type;}
    QString getClientType() { return clientType;}

    void enterClassifierMode(QString diameter);
    void enterSmpsClassifierMode(QString diameter);
    void enterAutoMode();

    int getValue(QList<QString> vals);
    int setValue(QMap<QString,QString> valueSet);

    int sendValues(QMap<int, QVariant>);
    void setMode(QList<QString> imodes) {modes = imodes;}
    QList<QString> getMode() {return modes;}

    void connectToHost(QString host, int port);
    void disconnectFromHost();
    void close();
    int  state();

signals:
    void sigReadData(QString type, QMap<QString,QString> values);
    void sigSetRet(QString type, QString ret, QMap<QString,QString> setValues);
    void sigConnected();
    void sigDisConnected();

private slots:
    void onDataReady();
    void onConnected();
    void onDisConnected();

private:
    QTcpSocket *sock;
    QString clientType = QString();
    QByteArray dataRemain;
    bool beSimu = false;
    QMap<QString,QString> devValueSet;
    QMap<QString,QString> sendValueSet;
    QList<QString> modes;
    QString mode;

private:
    void commDataProcess();
    void simuDataProcess();
};

#endif // CALIBCLIENT_H
