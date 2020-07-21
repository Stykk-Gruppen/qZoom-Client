#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QNetworkDatagram>
#include <QProcess>
#include <mutex>
#include "handlers/userhandler.h"

class SocketHandler : public QObject
{
    Q_OBJECT
public:
    explicit SocketHandler(std::mutex*,std::mutex*,UserHandler*,QObject *parent = nullptr);
    void initSocket();
    QTcpSocket* mTCPSocket;
    QUdpSocket* udpSocket;
    int sendDatagram(QByteArray arr);
    QHostAddress address;
    int port;
    QByteArray mAudioBuffer;
    QByteArray mVideoBuffer;
public slots:
    void readPendingDatagrams();
private:
    UserHandler* mUserHandler;
    int mInitialBufferSize = 4 * 1024;
    uint signalCount = 0;
    std::mutex *mAudioWriteLock;
    std::mutex *mVideoWriteLock;
    bool mVideoPlaybackStarted = false;
    bool mAudioPlaybackStarted = false;
signals:
    void startVideoPlayback();
    void startAudioPlayback();
};

#endif // SOCKETHANDLER_H
