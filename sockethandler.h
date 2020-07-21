#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QNetworkDatagram>
#include <QProcess>
#include <mutex>
#include "handlers/sessionhandler.h"
#include "imagehandler.h"
#include "videoplaybackhandler.h"
#include "audioplaybackhandler.h"

class SocketHandler : public QObject
{
    Q_OBJECT
public:
    explicit SocketHandler(int, ImageHandler*,SessionHandler*,QObject *parent = nullptr);
    void initSocket();
    QTcpSocket* mTCPSocket;
    QUdpSocket* udpSocket;
    int sendDatagram(QByteArray arr);
    QHostAddress address;
    int port;
public slots:
    void readPendingDatagrams();
private:
    void addStreamToVector(QString,int);
    int findStreamIdIndex(QString);
    std::vector<QString> mStreamIdVector;
    std::vector<QByteArray*> mAudioBufferVector;
    std::vector<QByteArray*> mVideoBufferVector;
    std::vector<std::mutex*> mAudioMutexVector;
    std::vector<std::mutex*> mVideoMutexVector;
    std::vector<AudioPlaybackHandler*> mAudioPlaybackHandlerVector;
    std::vector<VideoPlaybackHandler*> mVideoPlaybackHandlerVector;
    std::vector<bool> mVideoPlaybackStartedVector;
    std::vector<bool> mAudioPlaybackStartedVector;
    SessionHandler* mSessionHandler;
    ImageHandler* mImageHandler;
    int mBufferSize;
    uint signalCount = 0;
    struct mBufferAndLockStruct {
        QByteArray* buffer;
        std::mutex* writeLock;
    };
};

#endif // SOCKETHANDLER_H
