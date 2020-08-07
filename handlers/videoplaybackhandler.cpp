#include "videoplaybackhandler.h"

VideoPlaybackHandler::VideoPlaybackHandler(std::mutex* _writeLock, QByteArray* buffer,
                                           size_t bufferSize, ImageHandler* _imageHandler,int index,
                                           QObject *parent) : Playback(_writeLock, buffer, bufferSize, _imageHandler, index, parent)
{

}

VideoPlaybackHandler::~VideoPlaybackHandler()
{

}

static int interruptCallBack(void* ctx)
{
    qDebug() << "inside interrupt";
    return 0;
}

static const AVIOInterruptCB icb = {interruptCallBack, NULL};

void VideoPlaybackHandler::start()
{
    mStopPlayback = false;
    int error = 0;
    AVFormatContext *inputFormatContext = avformat_alloc_context();
    Q_ASSERT(inputFormatContext);


    inputFormatContext->interrupt_callback = icb;
    //inputFormatContext->interrupt_callback.callback = interruptCallBack;
    //inputFormatContext->interrupt_callback.opaque = NULL;

    uint8_t *avioContextBuffer = reinterpret_cast<uint8_t*>(av_malloc(mBufferSize));
    Q_ASSERT(avioContextBuffer);

    AVIOContext *avioContext = avio_alloc_context(avioContextBuffer, static_cast<int>(mBufferSize), 0, mStruct, &customReadPacket, nullptr, nullptr);
    Q_ASSERT(avioContext);

    inputFormatContext->pb = avioContext;

    error = avformat_open_input(&inputFormatContext, NULL, NULL, NULL);
    qDebug() << "HEADER RECEIVED" << Q_FUNC_INFO;

    if(error < 0)
    {
        char* errbuff = (char *)malloc((1000)*sizeof(char));
        av_strerror(error,errbuff,1000);
        qDebug() << "AVformat open input UDP stream failed" << errbuff;
        exit(1);
    }

    //Funker ikke med denne linja, men litt rart det funker uten?
    //error = avformat_find_stream_info(fmt_ctx, nullptr);
    /*if(error < 0)
        {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(error,errbuff,1000);
            qDebug() << "AVFormat find udp stream failed" << errbuff;
            exit(1);
        }*/

    qDebug() << "Dumping videoplayback format";
    av_dump_format(inputFormatContext, 0, NULL, 0);

    AVStream* video_stream = nullptr;
    for (uint i=0; i < inputFormatContext->nb_streams; ++i) {
        auto	st = inputFormatContext->streams[i];
        //Debug() << st->id << st->index << st->start_time << st->duration << st->codecpar->codec_type;
        if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream = st;
        }
    }

    int err;
    //Q_ASSERT(video_stream);
    AVCodecContext *videoDecoderCodecContext;
    if(video_stream)
    {
        // video_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        AVCodecParameters	*videoStreamCodecParameters = video_stream->codecpar;
        AVCodec* videoDecoderCodec = avcodec_find_decoder(videoStreamCodecParameters->codec_id);
        videoDecoderCodecContext = avcodec_alloc_context3(videoDecoderCodec);
        videoDecoderCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
        err = avcodec_open2(videoDecoderCodecContext, videoDecoderCodec, nullptr);
        Q_ASSERT(err>=0);
        qDebug() << "codec name: " << videoDecoderCodec->name<< " codec id " << videoDecoderCodec->id;
        qDebug() << "codecpar width" << videoStreamCodecParameters->width <<" h: "<< videoStreamCodecParameters->height << " format: "<< videoStreamCodecParameters->format<< " pix fmt: " << videoDecoderCodecContext->pix_fmt;

        AVFrame	*frameRGB = av_frame_alloc();
        frameRGB->format = AV_PIX_FMT_RGB24;
        frameRGB->width = videoStreamCodecParameters->width;
        frameRGB->height = videoStreamCodecParameters->height;
        err = av_frame_get_buffer(frameRGB, 0);
        Q_ASSERT(err == 0);
    }

    AVFrame* frame = av_frame_alloc();
    AVPacket* packet = av_packet_alloc();
    //AVFrame* resampled = 0;

    while (!mStopPlayback) {
        //qDebug() << "About to call av read frame";
        //av_read_frame(fmt_ctx, NULL);

        AVCodecContext *cctx;
        //inputFormatContext->
        //AVPacket *pkt;
        //AVFrame *frm;
        uint8_t recvbuf[(int)10e5];
        memset(recvbuf,0,10e5);
        int pos = 0;

        AVCodecParserContext * parser = av_parser_init(AV_CODEC_ID_H264);
        parser->flags |= PARSER_FLAG_COMPLETE_FRAMES;
        parser->flags |= PARSER_FLAG_USE_CODEC_TS;

        while (mStruct->buffer->size() <= mBufferSize)
        {
            if((*mStruct->stopPlayback))
            {
                //return AVERROR_EOF;
                break;
            }
            //int ms = 5;
            //struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
            //qDebug() << "sleeping";
            //nanosleep(&ts, NULL);
        }

        mStruct->writeLock->lock();
        QByteArray tempBuffer = QByteArray(mStruct->buffer->data(), mBufferSize);
        mStruct->buffer->remove(0, mBufferSize);
        mStruct->writeLock->unlock();

        memcpy(recvbuf, tempBuffer.constData(), mBufferSize);

        //int length = read(, recvbuf, 10e5);
        //int length = mStruct->buffer->size();

        int length = mBufferSize;
        if (length >= 0)
        {

            //Creating temporary packet
            AVPacket * tempPacket = new AVPacket;
            av_init_packet(tempPacket);
            av_new_packet(tempPacket, length);
            memcpy(tempPacket->data, recvbuf, length);
            tempPacket->pos = pos;
            pos += length;
            memset(recvbuf,0,length);

            //Parsing temporary packet into pkt
            av_init_packet(packet);
            av_parser_parse2(parser, videoDecoderCodecContext,
                &(packet->data), &(packet->size),
                tempPacket->data, tempPacket->size,
                tempPacket->pts, tempPacket->dts, tempPacket->pos
                );

            packet->pts = parser->pts;
            packet->dts = parser->dts;
            packet->pos = parser->pos;

            //Set keyframe flag
            if (parser->key_frame == 1 ||
                (parser->key_frame == -1 &&
                parser->pict_type == AV_PICTURE_TYPE_I))
                packet->flags |= AV_PKT_FLAG_KEY;
            if (parser->key_frame == -1 && parser->pict_type == AV_PICTURE_TYPE_NONE && (packet->flags & AV_PKT_FLAG_KEY))
                packet->flags |= AV_PKT_FLAG_KEY;
            packet->duration = 96000; //Same result as in av_read_frame()

            //Decode:
            error = avcodec_send_packet(videoDecoderCodecContext, packet);
            if (error == AVERROR_EOF || error == AVERROR(EOF))
            {
                qDebug() << "send packet sleep";
                int ms = 1000;
                struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
                nanosleep(&ts, NULL);
                continue;
            }
            else if(error < 0)
            {
                char* errbuff = (char *)malloc((1000)*sizeof(char));
                av_strerror(error,errbuff,1000);
                qDebug() << "Failed udp input avcodec_send_packet: code "<<error<< " meaning: " << errbuff;
                exit(1);

            }
            error = avcodec_receive_frame(videoDecoderCodecContext, frame);
            if (error == AVERROR(EAGAIN) || error == AVERROR_EOF){
                //skipped_frames++;
                qDebug() << "Skipped a Frame VideoPlaybackHandler";
                continue;
            }
            else if (error < 0) {
                char* errbuff = (char *)malloc((1000)*sizeof(char));
                av_strerror(error,errbuff,1000);
                qDebug() << "Failed avcodec_receive_frame: code "<<error<< " meaning: " << errbuff;
                exit(1);
            }

            mImageHandler->readImage(videoDecoderCodecContext, frame, mIndex);

            av_frame_unref(frame);
            av_packet_unref(packet);
            //Display frame
            //…
        }








































        inputFormatContext->flags |= AVFMT_FLAG_NONBLOCK;
        //qDebug() << inputFormatContext->interrupt_callback.callback;
        /*

        error = av_read_frame(inputFormatContext, &packet);
        //qDebug() << "AVREADFRAME: " << error;
        if(error < 0)
        {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(error,errbuff,1000);
            qDebug() << "Failed av_read_frame in videoplaybackhandler: code " << error << " meaning: " << errbuff;
            //int ms = 1000;
            //struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
            //nanosleep(&ts, NULL);
            continue;
        }
        */



        //qDebug() << "stream: " << packet.stream_index << " mvideostream: " << mVideoStreamIndex;

        //Decode and send to ImageHandler
        //qDebug() << "packet dts VideoPlaybackHandler: " << packet.dts;
        //qDebug() << "packet pts VideoPlaybackHandler: " << packet.pts;

    }

    avformat_close_input(&inputFormatContext);
    avcodec_free_context(&videoDecoderCodecContext);

}

void VideoPlaybackHandler::decreaseIndex()
{
    mIndex--;
}



