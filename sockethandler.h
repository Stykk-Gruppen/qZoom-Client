#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QNetworkDatagram>

class SocketHandler : public QObject
{
    Q_OBJECT
public:
    explicit SocketHandler(QObject *parent = nullptr);
    void initSocket();
    void readPendingDatagrams();
    QUdpSocket* udpSocket;

signals:

};

#endif // SOCKETHANDLER_H
