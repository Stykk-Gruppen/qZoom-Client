#ifndef VIDEOPLAYBACKHANDLER_H
#define VIDEOPLAYBACKHANDLER_H

#include <QObject>
#include <QByteArray>
#include <QtConcurrent/QtConcurrent>
#include "handlers/imagehandler.h"
#include "handlers/tcpsockethandler.h"
#include "core/playback.h"

class VideoPlaybackHandler : public Playback
{
public:
    VideoPlaybackHandler(std::mutex* writeLock, QByteArray* buffer,
                         size_t bufferSize, ImageHandler* mImageHandler,
                         int index);
    ~VideoPlaybackHandler();
    void start();
    void decreaseIndex();

private:
    int parsePacket(AVCodecParserContext* parser, AVPacket* packet, const int& length, AVCodecContext *videoDecoderCodecContext);
    uint8_t mRecvbuf[(int)10e5];

};

#endif // PLAYBACKHANDLER_H
