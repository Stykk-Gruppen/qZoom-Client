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

};

#endif // PLAYBACKHANDLER_H
