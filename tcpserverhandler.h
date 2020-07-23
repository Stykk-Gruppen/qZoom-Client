#ifndef TCPSERVERHANDLER_H
#define TCPSERVERHANDLER_H

#include <QObject>
//SKal lytte fra server, og opprette nye instanser av videoplaybackhandler og audioplaybackhandler basert på streamId og RoomId?
class TcpServerHandler : public QObject
{
    Q_OBJECT
public:
    explicit TcpServerHandler(QObject *parent = nullptr);

signals:

};

#endif // TCPSERVERHANDLER_H
