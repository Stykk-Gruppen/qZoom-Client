#ifndef AUDIOPLAYBACKHANDLER_H
#define AUDIOPLAYBACKHANDLER_H

#include <QObject>
#include <QByteArray>
#include <QtConcurrent/QtConcurrent>
#include <QAudioFormat>
#include <QAudioOutput>
extern "C" {
#include <ao/ao.h>
#include "libswresample/swresample.h"
}
#include "imagehandler.h"

class AudioPlaybackHandler : public QObject
{
    Q_OBJECT
public:
    AudioPlaybackHandler(std::mutex* writeLock, QByteArray*, int, QObject *parent = nullptr);
    void getStream();
    static int read_packet(void *opaque, uint8_t *buf, int buf_size);
    void start();
    int decodeAndPlay();
    int mAudioStreamIndex = -1;
private:
    int mVectorIndex;
    struct mBufferAndLockStruct {
        QByteArray* buffer;
        std::mutex* writeLock;
    };
    int mBufferSize;
    mBufferAndLockStruct* mStruct;
    void initAudio(QObject *parent);
    QByteArray mBuffer;
    QAudioFormat mAudioFormat;
    QAudioOutput* mpAudio;
    QIODevice* mpOut;
    uint8_t mSenderId = 0; //Value set to 0 just for testing.
    ImageHandler* mImageHandler;
signals:
};

#endif // PLAYBACKHANDLER_H
