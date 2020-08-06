#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <QObject>
#include <QtConcurrent/QtConcurrent>
extern "C" {
#include "libavformat/avformat.h"
}
#include "handlers/imagehandler.h"
class Playback : public QObject
{
    Q_OBJECT
public:
    explicit Playback(std::mutex* _writeLock, QByteArray* buffer, size_t bufferSize, ImageHandler* _imageHandler, int index, QObject *parent = nullptr);
    virtual void start(){};
    void stop();
    ~Playback();
protected:
    static int customReadPacket(void *opaque, uint8_t *buf, int buf_size);
    struct mBufferAndLockStruct {
        QByteArray* buffer;
        std::mutex* writeLock;
        bool* stopPlayback;
    };
    int mIndex;
    bool mStopPlayback = false;
    size_t mBufferSize;
    mBufferAndLockStruct* mStruct;
    ImageHandler* mImageHandler;
};

#endif // PLAYBACK_H
