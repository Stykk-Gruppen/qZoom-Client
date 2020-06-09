#ifndef CAMERATEST_H
#define CAMERATEST_H

#include <QObject>
#include <QCamera>
#include <QCameraInfo>
#include <QVideoProbe>
#include <QMediaPlayer>
#include <QMediaRecorder>
#include <QObject>
#include <QVideoFrame>
#include <QCameraImageCapture>
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


class CameraTest : public QObject
{
    Q_OBJECT

    typedef struct OutputStream {
            AVStream *st;
            AVCodecContext *enc;

            /* pts of the next frame that will be generated */
            int64_t next_pts;
            int samples_count;

            AVFrame *frame;
            AVFrame *tmp_frame;

            float t, tincr, tincr2;

            struct SwsContext *sws_ctx;
            struct SwrContext *swr_ctx;
        } OutputStream;

public:
    CameraTest(QString cDeviceName, QString aDeviceName, QObject* parent = 0);
    int init();

    void add_stream(OutputStream *ost, AVFormatContext *oc,
                                  AVCodec **codec,
                                  enum AVCodecID codec_id);
    OutputStream video_st = { 0 }, audio_st = { 0 };
    void grabFrames();
    AVOutputFormat *ofmt;
    AVFormatContext *ifmt_ctx, *ofmt_ctx, *fmt;
    AVCodecContext* c;
    QString cDeviceName;
    QString aDeviceName;
    AVDictionary *opt = NULL;
    int audioStream, videoStream;
    bool done;
    const char* filename = "nyTest.mp4";
public slots:
    void toggleDone();
};

#endif // CAMERATEST_H
