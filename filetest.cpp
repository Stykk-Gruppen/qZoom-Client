#include "filetest.h"
#define STREAM_DURATION   10.0
#define STREAM_FRAME_RATE 25 // 25 images/s
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P // default pix_fmt
#define SCALE_FLAGS SWS_BICUBIC
filetest::filetest(){

}
static AVStream *audioOutStream = NULL;
static AVStream *videoOutStream = NULL;
static AVFormatContext *videoDemuxer = NULL;
static AVFormatContext *audioDemuxer = NULL;
static AVFormatContext *muxer = NULL;
static AVCodecContext *videoDecoder = NULL, *audioDecoder;
static AVCodecContext *videoEncoder = NULL, *audioEncoder;
static SwrContext *swr;
static AVStream *video_stream = NULL, *audio_stream = NULL;
static const char *src_filename = NULL;

static FILE *video_dst_file = NULL;
static FILE *audio_dst_file = NULL;

static uint8_t *video_dst_data[4] = {NULL};

static int samples_count= 0;
static int video_stream_idx = -1, audio_stream_idx = -1;
static AVFrame *frame = NULL;
static AVPacket audioPacket;
static AVPacket videoPacket;

int filetest::customWriteFunction(void* opaque, uint8_t *buffer, int buffer_size)
{
    SocketHandler* socketHandler = reinterpret_cast<SocketHandler*>(opaque);
    unsigned char *c = static_cast<unsigned char *>(buffer);
    QByteArray send;
    send = QByteArray(reinterpret_cast<char*>(c), buffer_size);
    return socketHandler->sendDatagram(send);
}

static void resampleFrame(AVFrame *frame){
   /* AVFrame *tempFrame = NULL;
    tempFrame = av_frame_alloc();

    frame->nb_samples     = audioEncoder->frame_size;
    frame->format         = audioEncoder->sample_fmt;
    frame->channel_layout = audioEncoder->channel_layout;
    int64_t src_ch_layout = AV_CH_LAYOUT_MONO;
    int64_t dst_ch_layout = audioEncoder->channel_layout;
    int dst_nb_samples,ret;

    dst_nb_samples = av_rescale_rnd(swr_get_delay(swr, audioEncoder->sample_rate) + frame->nb_samples,
                                    audioEncoder->sample_rate,audioEncoder->sample_rate, AV_ROUND_UP);
    av_assert0(dst_nb_samples == frame->nb_samples);


    ret = swr_convert(swr,
                      tempFrame->extended_data, dst_nb_samples,
                      (const uint8_t **)frame->data, frame->nb_samples);
    if (ret < 0) {
        fprintf(stderr, "Error while converting\n");
        exit(1);
    }
    frame = tempFrame;
    av_frame_free(&tempFrame);

    frame->pts = av_rescale_q(samples_count, (AVRational){1, audioEncoder->sample_rate}, audioEncoder->time_base);
    samples_count += dst_nb_samples;*/

    AVFrame *tempFrame = NULL;
    tempFrame = av_frame_alloc();
    swr_convert_frame(swr,tempFrame,frame);
}

static int decode_packet(AVCodecContext *encoder,AVCodecContext *decoder,  AVPacket *pkt, enum AVMediaType type)
{
    int ret = 0;

    // submit the packet to the decoder
    ret = avcodec_send_packet(decoder, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error submitting a packet for decoding");
        return ret;
    }

    // get all the available frames from the decoder
    while (ret >= 0) {
        ret = avcodec_receive_frame(decoder, frame);
        if (ret < 0) {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)){
                return 0;
            }
            fprintf(stderr, "Error during decoding ");
            exit(1);
        }
        if(type==AVMEDIA_TYPE_AUDIO){
            resampleFrame(frame);
        }
        ret = avcodec_send_frame(encoder, frame);
        if(ret == AVERROR_EOF){
            qDebug() << "send frame end of file";
        } else if (ret == AVERROR(EAGAIN)){
            qDebug() << "send frame, no frame";
        }

        else if(ret < 0)
        {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(ret,errbuff,1000);
            qDebug() << "Error with send frame code:" << ret << " string: "<< errbuff <<"\n";
            exit(1);
        }
        av_frame_unref(frame);
        if (ret < 0)
            return ret;
    }
    av_packet_unref(pkt);
    while(ret >=0){
        ret = avcodec_receive_packet(encoder, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            return 0;
        }
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
        av_write_frame(muxer,pkt);
    }
    return ret;
}
static int check_sample_fmt(const AVCodec *codec, enum AVSampleFormat sample_fmt)
{
    const enum AVSampleFormat *p = codec->sample_fmts;

    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sample_fmt)
            return 1;
        p++;
    }
    return 0;
}
static int select_sample_rate(const AVCodec *codec)
{
    const int *p;
    int best_samplerate = 0;

    if (!codec->supported_samplerates)
        return 44100;

    p = codec->supported_samplerates;
    while (*p) {
        if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
            best_samplerate = *p;
        p++;
    }
    return best_samplerate;
}

/* select layout with the highest channel count */
static int select_channel_layout(const AVCodec *codec)
{
    const uint64_t *p;
    uint64_t best_ch_layout = 0;
    int best_nb_channels   = 0;

    if (!codec->channel_layouts)
        return AV_CH_LAYOUT_STEREO;

    p = codec->channel_layouts;
    while (*p) {
        int nb_channels = av_get_channel_layout_nb_channels(*p);

        if (nb_channels > best_nb_channels) {
            best_ch_layout    = *p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return best_ch_layout;
}

static int openDecoderContext(int *stream_idx,
                              AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type, AVStream * out_stream)
{
    int ret, stream_index;
    AVStream *st;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), src_filename);
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }
        //https://ffmpeg.org/doxygen/4.1/group__lavc__core.html#ga6d02e640ccc12c783841ce51d09b9fa7
        ret = avcodec_parameters_copy(out_stream->codecpar, st->codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters\n");
            exit(1);
        }
        out_stream->codecpar->codec_tag = 0;

        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(type));
            return ret;
        }

        /* Init the decoders */
        if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}



int filetest::main ()
{
    av_register_all();
    avcodec_register_all();
    avdevice_register_all();
    int ret = 0;
    AVInputFormat* videoInputFormat = av_find_input_format("v4l2");
    if(videoInputFormat == NULL)
    {
        qDebug() << "Not found videoFormat\n";
        exit(1);
    }
    AVInputFormat* audioInputFormat = av_find_input_format("alsa");
    if(videoInputFormat == NULL)
    {
        qDebug() << "Not found videoFormat\n";
        exit(1);
    }


    //https://ffmpeg.org/doxygen/4.1/avformat_8h.html#a6ddf3d982feb45fa5081420ee911f5d5
    avformat_alloc_output_context2(&muxer, NULL, "mpeg", NULL);
    if (!muxer) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        exit(1);
    }

    //https://ffmpeg.org/doxygen/4.1/group__lavf__core.html#gadcb0fd3e507d9b58fe78f61f8ad39827
    audioOutStream = avformat_new_stream(muxer, NULL);
    if (!audioOutStream) {
        fprintf(stderr, "Failed allocating output audio stream\n");
        ret = AVERROR_UNKNOWN;
        exit(1);
    }
    videoOutStream = avformat_new_stream(muxer, NULL);
    if (!videoOutStream) {
        fprintf(stderr, "Failed allocating output video stream\n");
        ret = AVERROR_UNKNOWN;
        exit(1);
    }




    /* open input file, and allocate format context */
    if (avformat_open_input(&videoDemuxer, "/dev/video0", videoInputFormat, NULL) < 0) {
        fprintf(stderr, "Could not open video stream");
        exit(1);
    }
    /* open input file, and allocate format context */
    if (avformat_open_input(&audioDemuxer, "default", audioInputFormat, NULL) < 0) {
        fprintf(stderr, "Could not open audio stream");
        exit(1);
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(videoDemuxer, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }
    if (avformat_find_stream_info(audioDemuxer, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }

    if (openDecoderContext(&video_stream_idx, &videoDecoder, videoDemuxer, AVMEDIA_TYPE_VIDEO,videoOutStream) >= 0) {
        video_stream = videoDemuxer->streams[video_stream_idx];

        /* allocate image where the decoded image will be put */
        /* width = video_dec_ctx->width;
        height = video_dec_ctx->height;
        pix_fmt = video_dec_ctx->pix_fmt;
        ret = av_image_alloc(video_dst_data, video_dst_linesize,
                             width, height, pix_fmt, 1);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate raw video buffer\n");
            goto end;
        }
        video_dst_bufsize = ret;*/
    }

    if (openDecoderContext(&audio_stream_idx, &audioDecoder, audioDemuxer, AVMEDIA_TYPE_AUDIO,audioOutStream) >= 0) {
        audio_stream = audioDemuxer->streams[audio_stream_idx];
    }

    /* dump input information to stderr */
    av_dump_format(videoDemuxer, 0, 0, 0);
    av_dump_format(audioDemuxer, 0, 0, 0);

    if (!audio_stream && !video_stream) {
        fprintf(stderr, "Could not find audio or video stream in the input, aborting\n");
        ret = 1;
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        exit(1);
    }
    // AUDIO ENCODER SETTINGS
    AVCodec *enc = NULL;
    /* find encoder context from muxer outformat codec */
    enc = avcodec_find_encoder(muxer->oformat->audio_codec);
    if (!enc) {
        fprintf(stderr, "Failed to find audio codec");
        exit(1);
    }
    /* Allocate a codec context for the decoder */
    audioEncoder = avcodec_alloc_context3(enc);
    if (!audioEncoder) {
        fprintf(stderr, "Failed to allocate audio codec context");
        exit(1);
    }
    /* put sample parameters */
    audioEncoder->bit_rate = 64000;

    /* check that the encoder supports s16 pcm input */
    //audioEncoder->sample_fmt = AV_SAMPLE_FMT_S16;
    audioEncoder->sample_fmt = AV_SAMPLE_FMT_S16;
    if (!check_sample_fmt(enc, audioEncoder->sample_fmt)) {
        fprintf(stderr, "Encoder does not support sample format %s",
                av_get_sample_fmt_name(audioEncoder->sample_fmt));
        exit(1);
    }

    /* select other audio parameters supported by the encoder */
    audioEncoder->sample_rate    = select_sample_rate(enc);
    audioEncoder->channel_layout = select_channel_layout(enc);
    audioEncoder->channels       = av_get_channel_layout_nb_channels(audioEncoder->channel_layout);
    swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_layout",  audioEncoder->channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", audioEncoder->channel_layout,  0);
    av_opt_set_int(swr, "in_sample_rate",     audioEncoder->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate",    audioEncoder->sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt",  AV_SAMPLE_FMT_S16, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLTP,  0);
    if ((ret = swr_init(swr)) < 0) {
        fprintf(stderr, "Failed to initialize the resampling context\n");
        exit(1);
    }
    ret = avcodec_open2(audioEncoder, enc, NULL);
    if(ret<0){
        fprintf(stderr, "Failed to open audio codec");
        exit(1);
    }
    // VIDEO ENCODER SETTINGS
    enc = NULL;
    /* find encoder context from muxer outformat codec */
    enc = avcodec_find_encoder(muxer->oformat->video_codec);
    if (!enc) {
        fprintf(stderr, "Failed to find video codec");
        exit(1);
    }
    videoEncoder = avcodec_alloc_context3(enc);
    if (!videoEncoder) {
        fprintf(stderr, "Failed to allocate audio codec context");
        exit(1);
    }
    /* put sample parameters */
    videoEncoder->bit_rate = 400000;
    /* resolution must be a multiple of two */
    videoEncoder->width = 352;
    videoEncoder->height = 288;
    /* frames per second */
    videoEncoder->time_base = (AVRational){1, 25};
    videoEncoder->framerate = (AVRational){25, 1};

    /* emit one intra frame every ten frames
       * check frame pict_type before passing frame
       * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
       * then gop_size is ignored and the output of encoder
       * will always be I frame irrespective to gop_size
       */
    videoEncoder->gop_size = 10;
    videoEncoder->max_b_frames = 1;
    videoEncoder->pix_fmt = AV_PIX_FMT_YUV420P;
    if (videoEncoder->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        /* just for testing, we also add B-frames */
        videoEncoder->max_b_frames = 2;
    }
    if (videoEncoder->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        /* Needed to avoid using macroblocks in which some coeffs overflow.
                 * This does not happen with normal video, it just happens here as
                 * the motion of the chroma plane does not match the luma plane. */
        videoEncoder->mb_decision = 2;
    }

    if (enc->id == AV_CODEC_ID_H264){
        av_opt_set(videoEncoder->priv_data, "preset", "veryfast", 0);
    }

    ret = avcodec_open2(videoEncoder, enc, NULL);
    if(ret<0){
        fprintf(stderr, "Failed to open video codec");
        exit(1);
    }
    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&audioPacket);
    audioPacket.data = NULL;
    audioPacket.size = 0;
    av_init_packet(&videoPacket);
    videoPacket.data = NULL;
    videoPacket.size = 0;


    ret = avio_open(&muxer->pb, "muxTest", AVIO_FLAG_WRITE);

    if (ret < 0) {
        fprintf(stderr, "Could not open output file");
        exit(1);
    }

    ret = avformat_write_header(muxer, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        exit(1);
    }
    int x = 0;
    //https://ffmpeg.org/doxygen/4.1/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
    while ( av_read_frame(audioDemuxer, &audioPacket)>= 0 && av_read_frame(videoDemuxer, &videoPacket)>=0) {
        if(x>100){
            break;
        }
        x++;
        ret = decode_packet(videoEncoder,videoDecoder, &videoPacket,AVMEDIA_TYPE_VIDEO);
        av_packet_unref(&videoPacket);
        if (ret < 0){
            qDebug() << "decode encode video failed";
            exit(1);
        }
        ret = decode_packet(audioEncoder,audioDecoder, &audioPacket,AVMEDIA_TYPE_AUDIO);
        av_packet_unref(&audioPacket);
        if (ret < 0){
            qDebug() << "decode encode audio failed";
            exit(1);
        }
    }


    /* flush the decoders */
    if (videoDecoder)
        decode_packet(videoEncoder,videoDecoder, NULL,AVMEDIA_TYPE_VIDEO);
    if (audioDecoder)
        decode_packet(audioEncoder,audioDecoder, NULL,AVMEDIA_TYPE_AUDIO);
    av_write_trailer(muxer);


    avcodec_free_context(&videoDecoder);
    avcodec_free_context(&audioDecoder);
    avformat_close_input(&videoDemuxer);
    if (video_dst_file)
        fclose(video_dst_file);
    if (audio_dst_file)
        fclose(audio_dst_file);
    av_frame_free(&frame);
    av_free(video_dst_data[0]);

    return ret < 0;
}
