#ifndef STREAMHANDLER_H
#define STREAMHANDLER_H
#include "videohandler.h"
#include "audiohandler.h"
#include "sockethandler.h"
#include <QObject>
#include "imagehandler.h"
#include "settings.h"
#include "handlers/errorhandler.h"

class StreamHandler : public QObject
{
    Q_OBJECT
public:

    StreamHandler(ImageHandler* _imageHandler, SocketHandler* _socketHandler, int buffer_size, Settings* settings, TcpSocketHandler* tcpSocketHandler, QObject *parent = nullptr);
    VideoHandler* mVideoHandler = nullptr;
    AudioHandler* mAudioHandler = nullptr;
    Q_INVOKABLE void record();
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void enableAudio();
    Q_INVOKABLE void enableVideo();
    Q_INVOKABLE void disableAudio();
    Q_INVOKABLE void disableVideo();
    Q_INVOKABLE QVariantList getAudioInputDevices();
    Q_INVOKABLE void changeAudioInputDevice(QString deviceName);
    Q_INVOKABLE QString getDefaultAudioInputDevice();

private:
    int64_t mTime;
    SocketHandler* mSocketHandler;
    TcpSocketHandler* mTcpSocketHandler;
    ImageHandler* mImageHandler;
    bool mAudioEnabled = true;
    bool mVideoEnabled = true;
    int mBufferSize;
    std::mutex mUDPSendDatagramMutexLock;
    QString mAudioDevice;
    QString mVideoDevice;
};

#endif // STREAMHANDLER_H
