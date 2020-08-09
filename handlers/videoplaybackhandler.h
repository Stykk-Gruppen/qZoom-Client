#ifndef VIDEOPLAYBACKHANDLER_H
#define VIDEOPLAYBACKHANDLER_H

#include <QObject>
#include <QByteArray>
#include <QtConcurrent/QtConcurrent>
#include "handlers/imagehandler.h"
#include <handlers/tcpsockethandler.h>
#include <playback.h>

class VideoPlaybackHandler : public Playback
{
public:
    VideoPlaybackHandler(std::mutex* writeLock, QByteArray* buffer,
                         size_t bufferSize, ImageHandler* mImageHandler,
                         int index);
    ~VideoPlaybackHandler();
    void start();
    void decreaseIndex();
    //static int interruptCallBack(void* ctx);
private:

};

#endif // PLAYBACKHANDLER_H
