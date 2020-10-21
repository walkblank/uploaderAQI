#ifndef UPLOADCLIENT_H
#define UPLOADCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>

class UploadClient : public QTcpSocket
{
    Q_OBJECT
public:
    explicit UploadClient(QObject *parent = nullptr);

private slots:
    void onDataRecv();

private:
    int  sendPacket(QString contentStr);
    int  getCRC(QString str);
};

#endif // UPLOADCLIENT_H
