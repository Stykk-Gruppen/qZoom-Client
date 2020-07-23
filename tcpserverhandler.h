#ifndef TCPSERVERHANDLER_H
#define TCPSERVERHANDLER_H

#include <QObject>

class TcpServerHandler : public QObject
{
    Q_OBJECT
public:
    explicit TcpServerHandler(QObject *parent = nullptr);

signals:

};

#endif // TCPSERVERHANDLER_H
