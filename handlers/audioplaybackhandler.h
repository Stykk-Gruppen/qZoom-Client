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
#include "libavutil/opt.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
}
#include "handlers/imagehandler.h"
#include "core/playback.h"

class AudioPlaybackHandler : public Playback
{
public:
    AudioPlaybackHandler(std::mutex* writeLock, QByteArray* buffer,
                         size_t bufferSize, ImageHandler* _imageHandler, int index);
    ~AudioPlaybackHandler();
    void start();
private:
    void initAudio(QObject *parent);
    QAudioFormat mAudioFormat;
    QAudioOutput* mpAudio;
    QIODevice* mpOut;
    static int customReadPacket(void *opaque, uint8_t *buf, int buf_size);
    int initFilterGraph(AVFilterGraph **graph, AVFilterContext **src,
                                        AVCodecContext *ctx,AVFilterContext **sink);

};

#endif // PLAYBACKHANDLER_H
