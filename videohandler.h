#ifndef VIDEOHANDLER_H
#define VIDEOHANDLER_H

#include <QUdpSocket>
#include <QObject>
#include <QVideoFrame>
#include <QtConcurrent/QtConcurrent>
#include <iostream>
#include <fstream>
#include "sockethandler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
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
#include "libavutil/common.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
}

class VideoHandler : public QObject
{
    Q_OBJECT
public:
    VideoHandler(QString cDeviceName, AVFormatContext* _ofmt_ctx, std::mutex* _writeLock, QObject* parent = 0);
    int init();
    void grabFrames();
    bool writeToFile = true;
    int numberOfFrames = 200;
    const char* filename = "nyTest.ismv";
    QString aDeviceName;
    QString cDeviceName;
    std::mutex* writeLock;

    static int custom_io_write(void* opaque, uint8_t *buffer, int buffer_size);

private:
    int skipped_frames = 0;
    std::ofstream outfile;
    AVCodecContext* inputVideoCodecContext;
    AVCodecContext* outputVideoCodecContext;
    AVFrame* videoFrame;
    AVFrame* scaledFrame;
    AVFormatContext *ifmt_ctx, *ofmt_ctx;
    AVCodec* inputVideoCodec;
    AVCodec* outputVideoCodec;
    int videoStream;
    SocketHandler *socketHandler;
    struct SwsContext* img_convert_ctx;
};
#endif // VideoHandler_H
