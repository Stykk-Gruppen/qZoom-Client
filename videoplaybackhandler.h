#ifndef VIDEOPLAYBACKHANDLER_H
#define VIDEOPLAYBACKHANDLER_H

#include <QObject>
#include <QByteArray>
#include <QtConcurrent/QtConcurrent>
#include "imagehandler.h"
#include <tcpsockethandler.h>
#include <tcpserverhandler.h>

class VideoPlaybackHandler : public QObject
{
    Q_OBJECT
public:
    VideoPlaybackHandler(std::mutex*, ImageHandler*, QByteArray* headerBuffer, QByteArray*, int, int, QObject *parent = nullptr);
    void getStream();
    static int read_packet(void *opaque, uint8_t *buf, int buf_size);
    void start();
    int mVideoStreamIndex = -1;
private:
    int mIndex;
    struct mBufferAndLockStruct {
        QByteArray* buffer;
        std::mutex* writeLock;
        bool* headerReceived;
        QByteArray* headerBuffer;

    };
    int mBufferSize;
    mBufferAndLockStruct* mStruct;
    QByteArray mBuffer;
    QIODevice* mpOut;
    uint8_t mSenderId = 0; //Value set to 0 just for testing.
    ImageHandler* mImageHandler;
signals:
};

#endif // PLAYBACKHANDLER_H
