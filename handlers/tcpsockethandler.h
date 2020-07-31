#ifndef TCPSOCKETHANDLER_H
#define TCPSOCKETHANDLER_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QHostAddress>
#include "handlers/videohandler.h"
#include <vector>
#include "handlers/inputstreamhandler.h"
#include <QString>
#include <QByteArray>

class InputStreamHandler;

class TcpSocketHandler : public QObject
{
    Q_OBJECT
public:
    explicit TcpSocketHandler(InputStreamHandler* inputStreamHandler, QString streamId, QString roomId, QString displayName, QHostAddress address, int port = 1338, QObject* parent = nullptr);
    void init();
    void close();
    QByteArray getReply();
    bool isReady();
    void wait();
    int getBytesWritten();
    void writeHeader();
    void updateDisplayName(QString displayName);
    void sendChangedDisplayNameSignal();
    QByteArray myHeader;

public slots:
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();

private:
    //Should match enum in Server::TcpServerHandler
    enum mTcpReturnValues { STREAM_ID_NOT_FOUND, ROOM_ID_NOT_FOUND, SESSION_STARTED };
    QHostAddress mAddress;
    int mPort;
    int mBytesWritten;
    QTcpSocket* mSocket;
    QByteArray mRequest;
    QByteArray mReply;
    bool mReady;
    QString mRoomId;
    QString mStreamId;
    QString mDisplayName;
    InputStreamHandler* mInputStreamHandler;
    std::vector<QByteArray> videoHeaders;
    void addStream();

};
#endif // TCPSOCKETHANDLER_H
