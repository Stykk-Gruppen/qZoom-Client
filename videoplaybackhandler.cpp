#include "videoplaybackhandler.h"

VideoPlaybackHandler::VideoPlaybackHandler(std::mutex* _writeLock,ImageHandler* _imageHandler, QByteArray* headerBuffer,
                                           QByteArray* buffer, int bufferSize, int index,
                                           //int64_t* lastVideoPacketTime, int64_t* lastAudioPacketTime,
                                           QObject *parent) : QObject(parent)
{
    mIndex = index;
    mBufferSize = bufferSize;
    mImageHandler = _imageHandler;
    mStruct = new mBufferAndLockStruct();
    mStruct->buffer = buffer;
    mStruct->writeLock = _writeLock;
    //Lagt til disse for å kunne få header via tcp
    mStruct->headerReceived = new bool(false);
    mStruct->headerBuffer = headerBuffer;

}

VideoPlaybackHandler::~VideoPlaybackHandler()
{
    delete mStruct;
}

int VideoPlaybackHandler::read_packet(void *opaque, uint8_t *buf, int buf_size)
{

   // qDebug() << "Inne i read packet";

    mBufferAndLockStruct *s = reinterpret_cast<mBufferAndLockStruct*>(opaque);

    //Kaller server for å få header via tcp
    QByteArray tempBuffer;

    if(0)//!(*s->headerReceived))
    {
        //while(s->headerBuffer->size() <= buf_size);
        buf_size = FFMIN(buf_size, s->headerBuffer->size());
        if(!buf_size)
        {
            qDebug() << AVERROR_EXIT;
            qDebug() << AVERROR_EOF;
            return AVERROR_EXIT;
        }
        s->writeLock->lock();
        //tempBuffer = QByteArray(s->headerBuffer->data(), buf_size);
        qDebug() << s->headerBuffer->length();
        qDebug() << buf_size;
        qDebug() << "HeaderBuffer: " << tempBuffer;

        s->headerBuffer->remove(0, buf_size);

        s->writeLock->unlock();

        memcpy(buf, tempBuffer.constData(), buf_size);

        //mSenderId = something;
        //qDebug() << "Reading packet";
        qDebug() << "Returning buf_size" << buf_size;
        return buf_size;
    }
    else
    {
        //buf_size = FFMIN(buf_size, s->socketHandler->mBuffer.size());

        while (s->buffer->size() <= buf_size)
        {
            /*int ms = 50;
            struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
            qDebug() << "sleeping";
            nanosleep(&ts, NULL);*/
            //qDebug() << "sleeping";

        }


        s->writeLock->lock();

        tempBuffer = QByteArray(s->buffer->data(), buf_size);
        s->buffer->remove(0,buf_size);
        //qDebug() << " buffer after removal: " << s->socketHandler->mBuffer.size();

        s->writeLock->unlock();

        memcpy(buf, tempBuffer.constData(), buf_size);

        //mSenderId = something;
        return buf_size;

    }

}

void VideoPlaybackHandler::start()
{

        AVFormatContext *fmt_ctx = nullptr;
        AVIOContext *avio_ctx = nullptr;
        uint8_t /**buffer = nullptr,*/ *avio_ctx_buffer = nullptr;
        size_t /*buffer_size = 0,*/ avio_ctx_buffer_size = mBufferSize;

        int ret = 0;
        fmt_ctx = avformat_alloc_context();
        Q_ASSERT(fmt_ctx);

        avio_ctx_buffer = reinterpret_cast<uint8_t*>(av_malloc(avio_ctx_buffer_size));
        Q_ASSERT(avio_ctx_buffer);
        avio_ctx = avio_alloc_context(avio_ctx_buffer, static_cast<int>(avio_ctx_buffer_size), 0, mStruct, &read_packet, nullptr, nullptr);
        Q_ASSERT(avio_ctx);

        fmt_ctx->pb = avio_ctx;
        ret = avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr);
        qDebug() << "HEADER RECEIVED";
        *mStruct->headerReceived = true;
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

        qDebug() << "Dumping videoplayback format";
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
        AVPacket packet;
        //AVFrame* resampled = 0;

        //Mulig vi må legge til en stop variabel her, slik at ting ikke krasher når vi forlater en rom.
        while (1) {
            //qDebug() << "About to call av read frame";
            //av_read_frame(fmt_ctx, NULL);
            ret = av_read_frame(fmt_ctx,&packet);
            //qDebug() << "AVREADFRAME: " << ret;
            if(ret < 0)
            {
                char* errbuff = (char *)malloc((1000)*sizeof(char));
                av_strerror(ret,errbuff,1000);
                qDebug() << "Failed av_read_frame in videoplaybackhandler: code " << ret << " meaning: " << errbuff;
                //int ms = 1000;
                //struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
                //nanosleep(&ts, NULL);
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

                //*mLastVideoPacketTime = frame->pts;
                //qDebug() << "VideoPacketTime: " << *mLastVideoPacketTime;
                //sync();

                //qDebug() << frame->data[0];
                //qDebug() << mIndex;
               // qDebug() << "Sending image to imageHandler";
                mImageHandler->readImage(videoDecoderCodecContext, frame, mIndex);
            }
            else
            {
                qDebug() << "Vi er fucked";
            }

            av_frame_unref(frame);
            av_packet_unref(&packet);
        }

    //Skal aldri komme hit

}

/*void VideoPlaybackHandler::sync()
{
    if(*mLastAudioPacketTime != -1)
    {
        int diff = *mLastAudioPacketTime - *mLastVideoPacketTime;
        if(diff < 0)
        {
            //int ms = abs(diff)/1000;
            int ms = 5;
           // qDebug() << "Video Sleeping: " << ms;

            struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
            nanosleep(&ts, NULL);
        }
    }
}*/

