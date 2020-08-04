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
    Q_OBJECT
public:
    VideoPlaybackHandler(std::mutex* writeLock, QByteArray* buffer,
                         size_t bufferSize, ImageHandler* imageHandler,
                         int index, QObject *parent = nullptr);
    ~VideoPlaybackHandler();
    void start();
    void decreaseIndex();
private:

};

#endif // PLAYBACKHANDLER_H
