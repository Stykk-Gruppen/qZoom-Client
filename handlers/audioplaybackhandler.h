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
#include "playback.h"

class AudioPlaybackHandler : public Playback
{
    Q_OBJECT
public:
    AudioPlaybackHandler(std::mutex* writeLock, QByteArray* buffer,
                         size_t bufferSize, QObject *parent = nullptr);
    ~AudioPlaybackHandler();
    void start();
private:
    void initAudio(QObject *parent);
    QAudioFormat mAudioFormat;
    QAudioOutput* mpAudio;
    QIODevice* mpOut;

    int init_filter_graph(AVFilterGraph **graph, AVFilterContext **src,
                                        AVCodecContext *ctx,AVFilterContext **sink);

};

#endif // PLAYBACKHANDLER_H
