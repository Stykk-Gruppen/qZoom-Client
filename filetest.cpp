#include "filetest.h"
#define STREAM_DURATION   10.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */
#define SCALE_FLAGS SWS_BICUBIC
filetest::filetest()
{

}
static AVStream *out_stream;
static AVFormatContext *fmt_ctx = NULL;
static AVFormatContext *ofmt_ctx = NULL;
static AVCodecContext *video_dec_ctx = NULL;
static AVStream *in_stream = NULL;
static const char *src_filename = NULL;
static const char *dst_filename = "final.mp4";


static int video_stream_idx = -1;
static AVFrame *frame = NULL;
static AVPacket pkt;





int filetest::main()
{
    int testCount = 0;

    int ret = 0;
    av_register_all();
    avcodec_register_all();
    avdevice_register_all();
    //video_dst_filename = "demuxedVideo.mp4";

    //STREAMS


    //Find input video formats
    AVInputFormat* videoInputFormat = av_find_input_format("v4l2");
    if(videoInputFormat == NULL)
    {
        fprintf(stderr,"Not found videoFormat\n");
        return -1;
    }
    //Open VideoInput
    if (avformat_open_input(&fmt_ctx, "/dev/video0", videoInputFormat, NULL) < 0) {
        fprintf(stderr, "Could not open input file '%s'", "/dev/video0");
        return -1;
    }

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, dst_filename);
    if (!ofmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(AVMEDIA_TYPE_VIDEO), src_filename);
        return ret;
    } else {
        AVCodec *dec = NULL;
        int stream_index = ret;
        AVStream *st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
            return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        video_dec_ctx = avcodec_alloc_context3(dec);
        if (!video_dec_ctx) {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(video_dec_ctx, st->codecpar)) < 0) {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
            return ret;
        }

        /* Init the decoders */
        if ((ret = avcodec_open2(video_dec_ctx, dec, NULL)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
            return ret;
        }
        video_stream_idx = stream_index;
    }


    in_stream = fmt_ctx->streams[video_stream_idx];
    // qDebug() << "Opened video codec context";

    //Kopiert fra remuxing.c
    out_stream = avformat_new_stream(ofmt_ctx, NULL);
    if (!out_stream) {
        fprintf(stderr, "Failed allocating output stream\n");
        exit(1);
    }
    ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
    if (ret < 0) {
        fprintf(stderr, "Failed to copy codec parameters\n");
        exit(1);
    }
    ret = avio_open(&ofmt_ctx->pb, dst_filename, AVIO_FLAG_WRITE);
    if (ret < 0) {
        fprintf(stderr, "Could not open output file '%s'", dst_filename);
        exit(1);
    }
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        exit(1);
    }




    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, src_filename, 0);
    //exit(1);

    if (!in_stream) {
        fprintf(stderr, "Could not find video stream in the input, aborting\n");
        ret = 1;
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    /* read x frames from the stream */
    while (av_read_frame(fmt_ctx, &pkt) >= 0 && testCount < 100) {
        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
        if (pkt.stream_index == video_stream_idx){

            testCount++;
            pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
            pkt.pos = -1;
            ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
            if (ret < 0) {
                fprintf(stderr, "Error muxing packet\n");
                exit(1);
            }
        }
        av_packet_unref(&pkt);
        if (ret < 0)
            break;
    }
    av_write_trailer(ofmt_ctx);
end:
    avcodec_free_context(&video_dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_frame_free(&frame);

    return ret < 0;
}

