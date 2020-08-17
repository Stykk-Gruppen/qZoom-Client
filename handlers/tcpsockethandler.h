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
    explicit TcpSocketHandler(InputStreamHandler* inputStreamHandler, QString streamId, QString roomId,
                              QString displayName, QHostAddress address, int port, QObject* parent = nullptr);
    ~TcpSocketHandler();
    void updateDisplayName(const QString& displayName);
    void sendChangedDisplayNameSignal();
    void sendDisabledVideoSignal();
    void sendDisabledAudioSignal();
    void sendKickParticipantSignal(const QString& streamId);
    void appendToHeader(const QByteArray& data);
    void close();
    int init();
    bool isOpen() const;
    QTcpSocket* getSocket();

signals:
    void sendDummyDatagram();

public slots:
    void sendVideoHeader();
    void bytesWritten(qint64 bytes);
    void readyRead();

private:
    void addStream();
    void prependDefaultHeader(QByteArray& data) const;
    int mPort;
    int mBytesWritten;
    bool mReady;
    QHostAddress mAddress;
    QTcpSocket* mSocket;
    QByteArray mRequest;
    QByteArray mReply;
    QByteArray mHeader;
    QString mRoomId;
    QString mStreamId;
    QString mDisplayName;
    InputStreamHandler* mInputStreamHandler;
    std::vector<QByteArray> videoHeaders;
    enum mTcpHeaderCodes { VIDEO_HEADER, REMOVE_PARTICIPANT, NEW_DISPLAY_NAME, VIDEO_DISABLED, AUDIO_DISABLED, KICK_PARTICIPANT };
};
#endif // TCPSOCKETHANDLER_H
