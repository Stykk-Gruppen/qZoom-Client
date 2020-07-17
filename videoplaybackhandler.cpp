#include "videoplaybackhandler.h"

VideoPlaybackHandler::VideoPlaybackHandler(std::mutex* writeLock,ImageHandler* _imageHandler, SocketHandler* _socketHandler, QObject *parent)
{
    mSocketHandler = _socketHandler;
    mImageHandler = _imageHandler;
    mStruct = new SocketAndIDStruct();
    mStruct->socketHandler = _socketHandler;
    mStruct->writeLock = writeLock;

    connect(mSocketHandler, &SocketHandler::startPlayback, this, &VideoPlaybackHandler::start);
}

int VideoPlaybackHandler::read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    SocketAndIDStruct *s = reinterpret_cast<SocketAndIDStruct*>(opaque);

    while (s->socketHandler->mBuffer.size() <= buf_size)
    {
        //int ms = 5;
        //struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        //qDebug() << "sleeping";
        //nanosleep(&ts, NULL);
    }

    s->writeLock->lock();

    QByteArray tempBuffer = QByteArray(s->socketHandler->mBuffer.data(), buf_size);
    s->socketHandler->mBuffer.remove(0,buf_size);

    s->writeLock->unlock();
    //qDebug() << " buffer after removal: " << s->socketHandler->mBuffer.size();


    memcpy(buf, tempBuffer.constData(), buf_size);

    //mSenderId = something;
    //qDebug() << "Reading packet";
    return buf_size;
}

int VideoPlaybackHandler::start()
{
    QtConcurrent::run([this]()
    {
        AVFormatContext *fmt_ctx = nullptr;
        AVIOContext *avio_ctx = nullptr;
        uint8_t *buffer = nullptr, *avio_ctx_buffer = nullptr;
        size_t buffer_size = 0, avio_ctx_buffer_size = 4*1024;

        int ret = 0;
        fmt_ctx = avformat_alloc_context();
        Q_ASSERT(fmt_ctx);

        avio_ctx_buffer = reinterpret_cast<uint8_t*>(av_malloc(avio_ctx_buffer_size));
        Q_ASSERT(avio_ctx_buffer);
        avio_ctx = avio_alloc_context(avio_ctx_buffer, static_cast<int>(avio_ctx_buffer_size), 0, mStruct, &read_packet, nullptr, nullptr);
        Q_ASSERT(avio_ctx);

        fmt_ctx->pb = avio_ctx;
        ret = avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr);
        if(ret < 0)
        {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(ret,errbuff,1000);
            qDebug() << "AVformat open input UDP stream failed" << errbuff;
            exit(1);
        }

        //Funker ikke med denne linja, men litt rart det funker uten?
        //ret = avformat_find_stream_info(fmt_ctx, nullptr);
        if(ret < 0)
        {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(ret,errbuff,1000);
            qDebug() << "AVFormat find udp stream failed" << errbuff;
            exit(1);
        }

        av_dump_format(fmt_ctx, 0, NULL, 0);

        AVStream* video_stream = nullptr;
        for (uint i=0; i < fmt_ctx->nb_streams; ++i) {
            auto	st = fmt_ctx->streams[i];
            //Debug() << st->id << st->index << st->start_time << st->duration << st->codecpar->codec_type;
            if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                video_stream = st;
                mVideoStreamIndex = i;
            }
        }

        int err;
        //Q_ASSERT(video_stream);
        AVCodecContext *videoDecoderCodecContext;
        if(video_stream)
        {
            video_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

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
        AVPacket packet;
        AVFrame* resampled = 0;

        while (1) {
            ret = av_read_frame(fmt_ctx,&packet);
            if(ret < 0)
            {
                char* errbuff = (char *)malloc((1000)*sizeof(char));
                av_strerror(ret,errbuff,1000);
                qDebug() << "Failed av_read_frame: code " << ret << " meaning: " << errbuff;
                int ms = 1000;
                struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
                nanosleep(&ts, NULL);
                continue;
            }
            //qDebug() << "stream: " << packet.stream_index << " mvideostream: " << mVideoStreamIndex;
            if(packet.stream_index == mVideoStreamIndex)
            {
                //Decode and send to ImageHandler
                //qDebug() << "packet dts VideoPlaybackHandler: " << packet.dts;
                //qDebug() << "packet pts VideoPlaybackHandler: " << packet.pts;
                ret = avcodec_send_packet(videoDecoderCodecContext, &packet);
                if (ret == AVERROR_EOF || ret == AVERROR(EOF))
                {
                    qDebug() << "send packet sleep";
                    int ms = 1000;
                    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
                    nanosleep(&ts, NULL);
                    continue;
                }
                else if(ret < 0)
                {
                    char* errbuff = (char *)malloc((1000)*sizeof(char));
                    av_strerror(ret,errbuff,1000);
                    qDebug() << "Failed udp input avcodec_send_packet: code "<<ret<< " meaning: " << errbuff;
                    exit(1);

                }
                ret = avcodec_receive_frame(videoDecoderCodecContext, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
                    //skipped_frames++;
                    qDebug() << "Skipped a Frame VideoPlaybackHandler";
                    continue;
                }
                else if (ret < 0) {
                    char* errbuff = (char *)malloc((1000)*sizeof(char));
                    av_strerror(ret,errbuff,1000);
                    qDebug() << "Failed avcodec_receive_frame: code "<<ret<< " meaning: " << errbuff;
                    exit(1);
                }

                //qDebug() << frame->data[0];
                mImageHandler->readImage(videoDecoderCodecContext, frame, 1);
            }
            else
            {
                qDebug() << "Vi er fucked";
            }

            av_frame_unref(frame);
            av_packet_unref(&packet);
        }
    });
    //Skal aldri komme hit
    return 0;
}

