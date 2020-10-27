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
    qint64 uploadMinData(QString minStr, double a18Value, double a19Value);
    qint64 uploadHourData(QString HourStr, double a18Value, double a19Value);

private slots:
    void onDataRecv();

private:
    qint64  sendPacket(QString contentStr);
    int  getCRC(QString str);
    bool setSystemDateTime(QDateTime dt);
    void parseContent(QString content);

    qint64 replyTimeSyncReq(bool setResult);
    qint64 replyUploadReq(QString beginTime, QString endTime);
};

#endif // UPLOADCLIENT_H
