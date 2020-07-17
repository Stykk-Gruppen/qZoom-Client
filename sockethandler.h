#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QProcess>
#include <mutex>

class SocketHandler : public QObject
{
    Q_OBJECT
public:
    explicit SocketHandler(std::mutex*,std::mutex*,QObject *parent = nullptr);
    void initSocket();

    QUdpSocket* udpSocket;
    int sendDatagram(QByteArray arr);
    QHostAddress address;
    int port;
    QByteArray mAudioBuffer;
    QByteArray mVideoBuffer;
public slots:
    void readPendingDatagrams();
private:
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
