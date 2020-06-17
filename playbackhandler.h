#ifndef PLAYBACKHANDLER_H
#define PLAYBACKHANDLER_H

#include <QObject>
#include <QMediaPlayer>
#include <QUdpSocket>
#include <QByteArray>
#include <QtConcurrent/QtConcurrent>
extern "C" {
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavdevice/avdevice.h>
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/frame.h"
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavutil/avutil.h"
#include "libavutil/dict.h"
#include "libavutil/eval.h"
#include "libavutil/fifo.h"
#include "libavutil/pixfmt.h"
#include "libavutil/rational.h"

#include "libswresample/swresample.h"
}

#include <QAudioFormat>
#include <QAudioOutput>

class PlaybackHandler : public QObject
{
    Q_OBJECT
public:
    PlaybackHandler(QObject *parent = nullptr);
    void getStream();
    int read_packet(void *opaque, uint8_t *buf, int buf_size);
    int start();
    int decodeAndPlay();
private:
    void initAudio(QObject *parent);

    QUdpSocket *mpUdpSocket;
    QByteArray mBuffer;
    QAudioFormat mAudioFormat;
    QAudioOutput* mpAudio;
    QIODevice* mpOut;
signals:
};

#endif // PLAYBACKHANDLER_H
