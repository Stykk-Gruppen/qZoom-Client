#ifndef VIDEOPLAYBACKHANDLER_H
#define VIDEOPLAYBACKHANDLER_H

#include <QObject>
#include <QMediaPlayer>
#include <QUdpSocket>
#include <QByteArray>
#include <QtConcurrent/QtConcurrent>
#include <QNetworkDatagram>

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
#include <ao/ao.h>
#include "libswresample/swresample.h"
}

#include <QAudioFormat>
#include <QAudioOutput>
#include "imagehandler.h"

class VideoPlaybackHandler : public QObject
{
    Q_OBJECT
public:
    VideoPlaybackHandler(std::mutex*, ImageHandler*, QByteArray*, int, int, QObject *parent = nullptr);
    void getStream();
    static int read_packet(void *opaque, uint8_t *buf, int buf_size);
    void start();
    int mVideoStreamIndex = -1;
private:
    int mIndex;
    struct mBufferAndLockStruct {
        QByteArray* buffer;
        std::mutex* writeLock;
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
