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
    ~TcpSocketHandler();
    void updateDisplayName(const QString& displayName);
    void sendChangedDisplayNameSignal();
    void sendDisabledVideoSignal();
    void sendDisabledAudioSignal();
    void sendKickParticipantSignal(const QString& streamId);
    void appendToHeader(const QByteArray& data);
    void close();
    void wait();
    int init();
    int getBytesWritten() const;
    bool isOpen() const;
    bool isReady() const;
    QByteArray getReply() const;
    QByteArray getHeader();
    QTcpSocket *getSocket();


public slots:
    void writeHeader();
    void connected();
    void disconnected();
    void bytesWritten(qint64 bytes);
    void readyRead();

private:
    //Should match enum in Server::TcpServerHandler
    enum mTcpReturnValues { STREAM_ID_NOT_FOUND, ROOM_ID_NOT_FOUND, SESSION_STARTED };
    enum mTcpHeaderCodes { VIDEO_HEADER, REMOVE_PARTICIPANT, NEW_DISPLAY_NAME, VIDEO_DISABLED, AUDIO_DISABLED, KICK_PARTICIPANT };
    QHostAddress mAddress;
    int mPort;
    int mBytesWritten;
    QTcpSocket* mSocket;
    QByteArray mRequest;
    QByteArray mReply;
    QByteArray mHeader;
    bool mReady;
    QString mRoomId;
    QString mStreamId;
    QString mDisplayName;
    InputStreamHandler* mInputStreamHandler;
    std::vector<QByteArray> videoHeaders;
    void addStream();

};
#endif // TCPSOCKETHANDLER_H
