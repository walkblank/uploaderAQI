#ifndef UPLOADCLIENT_H
#define UPLOADCLIENT_H

#include <QObject>
#include <QTcpSocket>

class UploadClient : public QTcpSocket
{
public:
    UploadClient(QObject *parent = nullptr);
};

#endif // UPLOADCLIENT_H
