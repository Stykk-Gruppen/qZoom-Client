#ifndef TCPSERVERHANDLER_H
#define TCPSERVERHANDLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>


//SKal lytte fra server, og opprette nye instanser av videoplaybackhandler og audioplaybackhandler basert på streamId og RoomId?
class TcpServerHandler : public QObject
{
    Q_OBJECT
public:
    explicit TcpServerHandler(QObject *parent = nullptr);

    void init();
    void wait();
    QByteArray getMessage();
signals:


private:
    QTcpServer* mTcpServer;
    QTcpSocket* tcpServerConnection = nullptr;
    void readTcpPacket();
    void acceptTcpConnection();
    int mPort;
    QByteArray message;
};

#endif // TCPSERVERHANDLER_H
