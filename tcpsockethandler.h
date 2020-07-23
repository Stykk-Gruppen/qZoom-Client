#ifndef TCPSOCKETHANDLER_H
#define TCPSOCKETHANDLER_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QHostAddress>

class TcpSocketHandler : public QObject
{
    Q_OBJECT
public:
    explicit TcpSocketHandler(QHostAddress address, QByteArray request, int port = 1338, QObject* parent = nullptr);
    void init();
    QByteArray getReply();
    bool isReady();
    void wait();
    int getBytesWritten();

public slots:
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();
private:
    QHostAddress mAddress;
    int mPort;
    int mBytesWritten;
    QTcpSocket* mSocket;
    QByteArray mRequest;
    QByteArray mReply;
    bool mReady;
};
#endif // TCPSOCKETHANDLER_H
