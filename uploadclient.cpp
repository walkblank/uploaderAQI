#include "uploadclient.h"

#include <QDateTime>
#include <windows.h>
#include <time.h>

UploadClient::UploadClient(AQIDataBaseMng *mng, QObject *parent)
    : mng(mng), QTcpSocket(parent)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(onDataRecv()));
}

qint64 UploadClient::uploadHourData(QString hourStr, double a18Value, double a19Value)
{
    QString a18Flag = "N";
    QString a19Flag = "N";
    QString uploadDataContent = QString("ST=22;CN=2061;PW=123456;MN=ATCS0001;CP=&&DataTime=%1;"
                                        "A18-Avg=%2,A18-flag=%3;A19-Avg=%4,A19-flag=%5&&")
            .arg(hourStr).arg(a18Value).arg(a18Flag).arg(a19Value).arg(a19Flag);
    qDebug()<<"hour data content" << uploadDataContent;
    return sendPacket(uploadDataContent);
}

qint64 UploadClient::replyHourQuery(QString qn, QString datatime, QString a18Value, QString a19Value)
{
    QString a18Flag = "N";
    QString a19Flag = "N";
    QString replyDataContent = QString("ST=22;CN=2062;PW=123456;MN=ATCS0001;CP=&&DataTime=%1;"
                                       "QN=%2;A18-Avg=%3,A18-flag=%4;A19-Avg=%5,A19-flag=%6&&")
                    .arg(datatime).arg(qn).arg(a18Value).arg(a18Flag).arg(a19Value).arg(a19Flag);
    return sendPacket(replyDataContent);
}

qint64 UploadClient::replyHourQueryEnd(QString cnt, QString ret)
{
    QString replyContent = QString("ST=91;CN=9012;PW=123456;MN=ATCS0001;CP=&&QN=%1;Cou=%2;ExeRtn=%3&&")
            .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz")).arg(cnt).arg(ret);

    return sendPacket(replyContent);
}

qint64 UploadClient::uploadMinData(QString minStr, double a18Value, double a19Value)
{
    QString a18Flag = "N";
    QString a19Flag = "N";
    QString uploadDataContent = QString("ST=22;CN=2011;PW=123456;MN=ATCS0001;CP=&&DataTime=%1;"
                                        "A18-Avg=%2,A18-flag=%3;A19-Avg=%4,A19-flag=%5&&")
            .arg(minStr).arg(a18Value).arg(a18Flag).arg(a19Value).arg(a19Flag);
    qDebug()<<"hour data content" << uploadDataContent;
    return sendPacket(uploadDataContent);
}

//##0097ST=22;CN=2061;PW=123456;MN=TJTYTYACFC0011;CP=&&DataTime=20160516130000;A18-Avg=40.28,A18-flag=N&&FE41
qint64 UploadClient::sendPacket(QString contentStr)
{
    QString crcCode = QString("%1").arg(getCRC(contentStr), 4, 16, QLatin1Char('0'));
    QString dataStr = QString("%1%2\r\n").arg(contentStr).arg(crcCode);
    QString length = QString("%1").arg(dataStr.length(), 4, 10, QLatin1Char('0'));
    QString sendDataStr = QString("##%1%2").arg(length).arg(dataStr);

    return write(sendDataStr.toStdString().c_str());
}

qint64 UploadClient::replyTimeSyncReq(bool setResult)
{
    QString replyContent = QString("ST=91;CN=1012;PW=123456;MN=ATCS0001;CP=&&QN=%1;ExeRtn=%2&&")
            .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz")).arg(setResult ? 1 : 0);
    qDebug()<<"replyContent"<< replyContent;
    return sendPacket(replyContent);
}

qint64 UploadClient::replyUploadReq(QString qn, QString beginTime, QString endTime)
{
    QString filter = QString("timestamp > \"%1\" and timestamp < \"%2\"")
            .arg(QDateTime::fromString(beginTime, "yyyyMMddhhmmss").addSecs(-3600).toString("yyyyMMddhhmmss"))
            .arg(QDateTime::fromString(endTime, "yyyyMMddhhmmss").addSecs(3600).toString("yyyyMMddhhmmss"));
    mng->setCurrFilter(filter);
    QSqlTableModel *tempModel = mng->getTableModel();
    int cnt = tempModel->rowCount();
    for(int i = 0; i < cnt; i ++)
    {
        qDebug()<<"record " << i << tempModel->record(i).value("timestamp").toString();
        replyHourQuery(qn, tempModel->record(i).value("timestamp").toString(),
                       tempModel->record(i).value("a18").toString(), tempModel->record(i).value("a19").toString());
    }

    return replyHourQueryEnd(QString("%1").arg(cnt), (cnt == 0) ? "1" : "100");
}

void UploadClient::parseContent(QString content)
{
    qDebug()<<"content";
    int length = content.mid(0, 4).toUInt(0, 16);
    QString crc =  content.mid(content.length()-6, 4);
    qDebug()<<"crc" << crc;
    qDebug()<<"length" << length;
    qDebug()<<"content" << content;

    QStringList fields;
    fields = content.split(";");
    QString cmd, data, qn;
    foreach(QString field, fields)
    {
        qDebug()<<field;
        if(field.contains("CN="))
            cmd = field.split("=")[1];
        if(field.contains("CP=&&"))
            data = field.split("&&")[1];
        if(field.contains("QN="))
            qn = field.split("=")[1];
    }

    qDebug()<<"cmd" << cmd << "\r\ndata" << data;
    if(cmd == "1012") // sync time
    {
        QDateTime sTime = QDateTime::fromString(data.split("=")[1], "yyyyMMddhhmmsszzz");
        qDebug()<<"time" << sTime;
        qDebug()<<"time" << sTime.time().hour();

        replyTimeSyncReq(setSystemDateTime(sTime));
    }
    if(cmd == "2062") //
    {
        qDebug()<<"upload request";
        QString beginTime, endTime;
        foreach(QString time,  data.split(","))
        {
            if(time.contains("BeginTime="))
                beginTime = time.split("=")[1];
            else if(time.contains("EndTime"))
                endTime = time.split("=")[1];
        }
        qDebug()<<"beginTime" << beginTime << "endTime" << endTime << "qn" << qn;
        replyUploadReq(qn, beginTime, endTime);
    }
}

void UploadClient::onDataRecv()
{
    static QString strLeft = QString();
    QString dataRecv = readAll();
    qDebug()<<"dataRecv" << dataRecv.size() << dataRecv;
    strLeft.append(dataRecv);
    qDebug()<<"str after recv" << strLeft;

    while(strLeft.contains("\r\n"))
    {
        int endPos = strLeft.indexOf("\r\n");

        if(strLeft.contains("##"))
        {
            int startPos = strLeft.indexOf("##");
            qDebug()<<"endPos" << endPos << "startPos" << startPos;
            // ##0097
            QString content = strLeft.mid(startPos+2, endPos-startPos);
            parseContent(content);
        }

        strLeft = strLeft.mid(endPos+2, strLeft.length()-endPos);
        qDebug()<<"now strLeft" << strLeft;
    }
}

bool UploadClient::setSystemDateTime(QDateTime dt)
{
    SYSTEMTIME systm;
    systm.wYear = dt.date().year();
    systm.wMonth = dt.date().month();
    systm.wDay = dt.date().day();
    systm.wHour = dt.time().hour();
    systm.wMinute = dt.time().minute();
    systm.wSecond = dt.time().second();
    //systm.wMilliseconds = dt.time().msec();

    return SetLocalTime(&systm);
}

int  UploadClient::getCRC(QString dataStr)
{
    std::string data212 = dataStr.toStdString();
    int CRC = 0xFFFF;
    int Num = 0xA001;
    int inum = 0;

    for (unsigned int j = 0; j < data212.size(); j++)
    {
        inum = data212[j];
        CRC = (CRC >> 8) & 0x00FF;
        CRC ^= inum;

        for (int k = 0; k < 8; k++)
        {
            int flag = CRC % 2;
            CRC = CRC >> 1;

            if (flag == 1)
                CRC = CRC ^ Num;
        }
    }
    return CRC;
}
