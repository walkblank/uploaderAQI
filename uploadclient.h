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
    int uploadMinData(QString minStr, double a18Value, double a19Value);
    int uploadHourData(QString HourStr, double a18Value, double a19Value);

private slots:
    void onDataRecv();

private:
    int  sendPacket(QString contentStr);
    int  getCRC(QString str);
};

#endif // UPLOADCLIENT_H
