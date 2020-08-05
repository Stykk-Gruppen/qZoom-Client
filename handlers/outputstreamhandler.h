#ifndef STREAMHANDLER_H
#define STREAMHANDLER_H
#include "handlers/videohandler.h"
#include "handlers/audiohandler.h"
#include "handlers/udpsockethandler.h"
#include <QObject>
#include "handlers/imagehandler.h"
#include "settings.h"
#include "handlers/errorhandler.h"

class OutputStreamHandler : public QObject
{
    Q_OBJECT
public:
    OutputStreamHandler(ImageHandler* _imageHandler, UdpSocketHandler* _socketHandler, size_t buffer_size, Settings* settings, TcpSocketHandler* tcpSocketHandler, QObject *parent = nullptr);
    VideoHandler* mVideoHandler = nullptr;
    AudioHandler* mAudioHandler = nullptr;
    Q_INVOKABLE void disableAudio();
    Q_INVOKABLE void disableVideo();
    Q_INVOKABLE void changeAudioInputDevice(QString deviceName);
    Q_INVOKABLE void init();
    Q_INVOKABLE QVariantList getAudioInputDevices();
    Q_INVOKABLE QString getDefaultAudioInputDevice();
    Q_INVOKABLE bool checkVideoEnabled();
    Q_INVOKABLE bool checkAudioEnabled();
    Q_INVOKABLE int enableAudio();
    Q_INVOKABLE int enableVideo(bool screenShare = false);

    void close();

private:
    QFuture<void> videoFuture;
    QFuture<void> audioFuture;
    void grabVideoHeader();
    int64_t mTime;
    UdpSocketHandler* mSocketHandler;
    TcpSocketHandler* mTcpSocketHandler;
    ImageHandler* mImageHandler;
    bool mAudioEnabled = true;
    bool mVideoEnabled = true;
    size_t mBufferSize;
    std::mutex mUDPSendDatagramMutexLock;
    QString mAudioDevice;
    QString mVideoDevice;
    Settings* mSettings;
};

#endif // STREAMHANDLER_H
