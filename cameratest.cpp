#include "cameratest.h"
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

CameraTest::CameraTest(QString cDeviceName, QString aDeviceName, QObject* parent): QObject(parent)
{
    done = false;
    this->cDeviceName = cDeviceName;
    this->aDeviceName = aDeviceName;
    av_register_all();
    avcodec_register_all();
    avdevice_register_all();
}

void CameraTest::toggleDone() {
    done = !done;
}

int CameraTest::init() {
    ofmt = NULL;
    ifmt_ctx = NULL;
    ofmt_ctx = NULL;

    QString fullDName = cDeviceName;// + ":" + aDeviceName;
    qDebug() << fullDName;
    AVInputFormat *fmt = av_find_input_format("v4l2");
    int ret, i;

    if (avformat_open_input(&ifmt_ctx, fullDName.toUtf8().data(), fmt, NULL) < 0) {
       fprintf(stderr, "Could not open input file '%s'", fullDName.toUtf8().data());
       return -1;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
       fprintf(stderr, "Failed to retrieve input stream information");
       return -1;
    }
    av_dump_format(ifmt_ctx, 0, fullDName.toUtf8().data(), 0);
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, "test.avi");
    if (!ofmt_ctx) {
       fprintf(stderr, "Could not create output context\n");
       ret = AVERROR_UNKNOWN;
       return -1;
    }
    ofmt = ofmt_ctx->oformat;

    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
       AVStream *in_stream = ifmt_ctx->streams[i];
       AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);

       if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
           videoStream = i;
       }
       else if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
           audioStream = i;
       }

       if (!out_stream) {
           fprintf(stderr, "Failed allocating output stream\n");
           ret = AVERROR_UNKNOWN;
           return -1;
       }
       ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
       if (ret < 0) {
           fprintf(stderr, "Failed to copy context from input to output stream codec context\n");
           return -1;
       }
       out_stream->codecpar->codec_tag = 0;
       if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
           out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    av_dump_format(ofmt_ctx, 0, "test.avi", 1);
    if (!(ofmt->flags & AVFMT_NOFILE)) {
       ret = avio_open(&ofmt_ctx->pb, "test.avi", AVIO_FLAG_WRITE);
       if (ret < 0) {
           fprintf(stderr, "Could not open output file '%s'", "test.avi");
           return -1;
       }
    }
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
       fprintf(stderr, "Error occurred when opening output file\n");
       return -1;
    }

    QtConcurrent::run(this, &CameraTest::grabFrames);
    QTimer::singleShot(3000, this, SLOT(toggleDone()));
    return 0;
}

void CameraTest::grabFrames() {
    AVPacket pkt;
    int ret;
    while (av_read_frame(ifmt_ctx, &pkt) >= 0) {
        AVStream *in_stream, *out_stream;
        in_stream  = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        /* copy packet */
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding) (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding) (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        int ret = av_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
           qDebug() << "Error muxing packet";
           //break;
        }
        av_free_packet(&pkt);

        if(done) break;
    }
    av_write_trailer(ofmt_ctx);

    avformat_close_input(&ifmt_ctx);
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
       avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF) {
        //return -1;
       //fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
    }

    qDebug() << "Ferdig med grabFrames!!!\n";
}
