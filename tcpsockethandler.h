#ifndef TCPSOCKETHANDLER_H
#define TCPSOCKETHANDLER_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QHostAddress>
#include "videohandler.h"
#include <vector>
#include "inputstreamhandler.h"

class InputStreamHandler;

class TcpSocketHandler : public QObject
{
    Q_OBJECT
public:
    explicit TcpSocketHandler(InputStreamHandler* inputStreamHandler, SessionHandler* sessionHandler, QHostAddress address, int port = 1338, QObject* parent = nullptr);
    void init();
    QByteArray getReply();
    bool isReady();
    void wait();
    int getBytesWritten();
    void writeHeader();
    QByteArray myHeader;
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
    SessionHandler* mSessionHandler;

    InputStreamHandler* mInputStreamHandler;
    std::vector<QByteArray> videoHeaders;


    void addStream();

};
#endif // TCPSOCKETHANDLER_H
