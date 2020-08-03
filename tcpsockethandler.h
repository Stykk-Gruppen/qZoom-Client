#ifndef TCPSOCKETHANDLER_H
#define TCPSOCKETHANDLER_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QHostAddress>
#include "videohandler.h"
#include <vector>
#include "inputstreamhandler.h"
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
    void sendDisabledVideoSignal();
    void sendDisabledAudioSignal();
    QByteArray myHeader;

public slots:
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();

private:
    //Should match enum in Server::TcpServerHandler
    enum mTcpReturnValues { STREAM_ID_NOT_FOUND, ROOM_ID_NOT_FOUND, SESSION_STARTED };
    enum mTcpHeaderCodes { VIDEO_HEADER, REMOVE_PARTICIPANT, NEW_DISPLAY_NAME, VIDEO_DISABLED, AUDIO_DISABLED };
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
