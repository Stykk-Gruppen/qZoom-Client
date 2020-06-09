#include "cameratest.h"
#include <QtConcurrent/QtConcurrent>
#include <QDebug>




#include "videohandler.h"
//#define STREAM_DURATION   5.0
#define STREAM_FRAME_RATE 10 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */
#define SCALE_FLAGS SWS_BICUBIC
#define QFRAMES_PER_MOVIE 100












CameraTest::CameraTest(QString cDeviceName, QString aDeviceName, QObject* parent): QObject(parent)
{
    done = false;
    this->cDeviceName = cDeviceName;
    this->aDeviceName = aDeviceName;
    //c->pix_fmt = STREAM_PIX_FMT;

}

void CameraTest::toggleDone() {
    done = !done;
}

int CameraTest::init() {

    av_register_all();
    avcodec_register_all();
    avdevice_register_all();

    ofmt = NULL;
    ifmt_ctx = NULL;
    ofmt_ctx = NULL;

    AVInputFormat* videoInputFormat = av_find_input_format("v4l2");

    if(videoInputFormat == NULL)
    {
        qDebug() << "Not found videoFormat\n";
        return -1;
    }

    AVInputFormat* audioInputFormat = av_find_input_format("alsa");

    if(!(audioInputFormat != NULL))
    {
        qDebug() << "Not found audioFormat\n";
        return -1;
    }

    int ret, i;


    if (avformat_open_input(&ifmt_ctx, cDeviceName.toUtf8().data(), videoInputFormat, NULL) < 0) {
       fprintf(stderr, "Could not open input file '%s'", cDeviceName.toUtf8().data());
       return -1;
    }

    if(avformat_open_input(&ifmt_ctx, aDeviceName.toUtf8().data(), audioInputFormat, NULL) < 0)
    {
        fprintf(stderr, "Could not open audio input file '%s'", aDeviceName.toUtf8().data());
        return -1;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
       fprintf(stderr, "Failed to retrieve input stream information");
       return -1;
    }
    av_dump_format(ifmt_ctx, 0, NULL, 0);






    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
    if (!ofmt_ctx) {
       fprintf(stderr, "Could not create output context\n");
       ret = AVERROR_UNKNOWN;
       return -1;
    }
    ofmt = ofmt_ctx->oformat;

    ofmt = av_guess_format(NULL, filename, NULL);


    AVCodec* videoCodec = avcodec_find_encoder(ofmt->video_codec);
    AVCodec* audioCodec = avcodec_find_encoder(ofmt->audio_codec);

    AVCodecContext* cv = avcodec_alloc_context3(videoCodec);

    qDebug() << cv->width;

    video_st.enc = cv;
    AVCodecContext* ca = avcodec_alloc_context3(audioCodec);
    audio_st.enc = ca;

    for (i = 0; (unsigned int)i < ifmt_ctx->nb_streams; i++) {
       AVStream *in_stream = ifmt_ctx->streams[i];
       AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);

       if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
           videoStream = i;
           cv->bit_rate = 400000;
           cv->width = in_stream->codecpar->width;
           cv->height = in_stream->codecpar->height;
           cv->pix_fmt = AV_PIX_FMT_YUV420P;
           avcodec_parameters_from_context(out_stream->codecpar, cv);
           qDebug() << "Fant en videoStream\n";


           img_convert_ctx = sws_getContext(
                        in_stream->codecpar->width,
                       in_stream->codecpar->height,
                       in_stream->codec->pix_fmt,
                       cv->width,
                       cv->height,
                       cv->pix_fmt,
                       SWS_BICUBIC,
                       NULL, NULL, NULL);
       }
       else if (ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
           audioStream = i;
           ca->sample_rate = 48000;
           ca->bit_rate = 96000;
           ca->channels = 2;
           ca->channel_layout = av_get_default_channel_layout(2);
           ca->frame_size = in_stream->codecpar->frame_size;

           /* Set the sample rate for the container. */
           out_stream->time_base.den = 48000;
           out_stream->time_base.num = 1;

           avcodec_parameters_from_context(out_stream->codecpar, ca);
           //avcodec_parameters_copy(out_stream->codecpar,in_stream->codecpar);
           qDebug() << "Fant en audioStream\n";
       }

       if (!out_stream) {
           fprintf(stderr, "Failed allocating output stream\n");
           ret = AVERROR_UNKNOWN;
           return -1;
       }

       //ret = copy_stream_props(out_stream,in_stream);
       //ret = avcodec_copy_context(out_stream->codecpar, in_stream->codecpar);
       /*if (ret < 0) {
           fprintf(stderr, "Failed to copy context from input to output stream codec context\n");
           return -1;
       }
       */

       out_stream->codec->codec_tag = 0;
       if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
           out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    av_dump_format(ofmt_ctx, 0, filename, 1);
    if (!(ofmt->flags & AVFMT_NOFILE)) {
       ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
       if (ret < 0) {
           fprintf(stderr, "Could not open output file '%s'", filename);
           return -1;
       }
    }

    //ofmt_ctx->streams[videoStream]->codec->pix_fmt = AV_PIX_FMT_YUV420P;


    av_dump_format(ofmt_ctx, 0, filename, 1);

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

        if(pkt.stream_index == videoStream)
        {
            //avcodec_decode_video2(ifmt_ctx->streams[videoStream]->codec, )
        }


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

