#ifndef TCPSERVERHANDLER_H
#define TCPSERVERHANDLER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "handlers/inputstreamhandler.h"

//SKal lytte fra server, og opprette nye instanser av videoplaybackhandler og audioplaybackhandler basert på streamId og RoomId?
class TcpServerHandler : public QObject
{
    Q_OBJECT
public:
    explicit TcpServerHandler(InputStreamHandler* inputStreamHandler, int port, QObject *parent = nullptr);

    void init();
    void close();
    void wait();
    QByteArray getMessage();
signals:


private:
    QTcpServer* mTcpServer;
    QTcpSocket* mTcpServerConnection = nullptr;
    void readTcpPacket();
    void acceptTcpConnection();
    int mPort;
    QByteArray message;
    InputStreamHandler* mInputStreamHandler;

};

#endif // TCPSERVERHANDLER_H
