#ifndef UPLOADCLIENT_H
#define UPLOADCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include "databasemng.h"

class UploadClient : public QTcpSocket
{
    Q_OBJECT
public:
    explicit UploadClient(AQIDataBaseMng *mng, QObject *parent = nullptr);
    qint64 uploadMinData(QString minStr, double a18Value, double a19Value);
    qint64 uploadHourData(QString HourStr, double a18Value, double a19Value);
    qint64 replyHourQuery(QString qn , QString datatime, QString a18Value, QString a19Vlaue);
    qint64 replyHourQueryEnd(QString cnt, QString ret);

private slots:
    void onDataRecv();

private:
    int  getCRC(QString str);
    bool setSystemDateTime(QDateTime dt);
    void parseContent(QString content);

    qint64 replyTimeSyncReq(bool setResult);
    qint64 replyUploadReq(QString qn, QString beginTime, QString endTime);
    qint64 sendPacket(QString contentStr);

private:
    AQIDataBaseMng *mng = nullptr;
};

#endif // UPLOADCLIENT_H
