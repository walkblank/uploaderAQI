#include "uploadclient.h"

UploadClient::UploadClient(QObject *parent)
    : QTcpSocket(parent)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(onDataRecv()));
}

//##0097ST=22;CN=2061;PW=123456;MN=TJTYTYACFC0011;CP=&&DataTime=20160516130000;A18-Avg=40.28,A18-flag=N&&FE41
int UploadClient::sendPacket(QString contentStr)
{
    QString crcCode = QString("%1").arg(getCRC(contentStr), 4, 16, QLatin1Char('0'));
    QString dataStr = QString("%1%2\r\n").arg(contentStr).arg(crcCode);
    QString length = QString("%1").arg(dataStr.length(), 4, 10, QLatin1Char('0'));
    QString sendDataStr = QString("##%1%2").arg(length).arg(dataStr);

    return write(sendDataStr.toStdString().c_str());
}

void UploadClient::onDataRecv()
{
}

int  UploadClient::getCRC(QString dataStr)
{
    std::string data212 = dataStr.toStdString();
    int CRC = 0xFFFF;
    int Num = 0xA001;
    int inum = 0;
    for (int j = 0; j < data212.size(); j++)
    {
        inum = data212[j];
        CRC = (CRC >> 8) & 0x00FF;
        CRC ^= inum;

        for (int k = 0; k < 8; k++)
        {
            int flag = CRC % 2;
            CRC = CRC >> 1;

            if (flag == 1)
            {
                CRC = CRC ^ Num;
            }
        }
    }
    return CRC;
}
