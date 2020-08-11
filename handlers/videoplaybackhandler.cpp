#include "videoplaybackhandler.h"
#define 	CODEC_CAP_TRUNCATED   0x0008
#define     CODEC_FLAG_TRUNCATED 0x00010000

VideoPlaybackHandler::VideoPlaybackHandler(std::mutex* _writeLock, QByteArray* buffer,
                                           size_t bufferSize, ImageHandler* _imageHandler,
                                           int index) : Playback(_writeLock, buffer, bufferSize, _imageHandler, index)
{

}

VideoPlaybackHandler::~VideoPlaybackHandler()
{

}


void VideoPlaybackHandler::start()
{
    mStopPlayback = false;
    int error = 0;
    AVFormatContext *inputFormatContext = avformat_alloc_context();
    Q_ASSERT(inputFormatContext);

   // mBufferSize = 10e5;
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
    for (uint i = 0; i < inputFormatContext->nb_streams; ++i)
    {
        AVStream* st = inputFormatContext->streams[i];
        //Debug() << st->id << st->index << st->start_time << st->duration << st->codecpar->codec_type;
        if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream = st;
        }
    }

    int err;
    //Q_ASSERT(video_stream);
    AVCodecContext *videoDecoderCodecContext = video_stream->codec;
    if(video_stream)
    {
        // video_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        AVCodecParameters	*videoStreamCodecParameters = video_stream->codecpar;
        AVCodec* videoDecoderCodec = avcodec_find_decoder(videoStreamCodecParameters->codec_id);

        if (videoDecoderCodec->capabilities & CODEC_CAP_TRUNCATED)
        {
            videoDecoderCodecContext->flags |= CODEC_FLAG_TRUNCATED;
        }
        videoDecoderCodecContext->thread_type  = FF_THREAD_SLICE;
        videoDecoderCodecContext->thread_count = 2;

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

    //uint8_t recvbuf[(int)10e5];
    memset(mRecvbuf, 0, 10e5);
    int pos = 0;

    AVCodecParserContext * parser = av_parser_init(AV_CODEC_ID_H264);
    parser->flags |= PARSER_FLAG_COMPLETE_FRAMES;
    parser->flags |= PARSER_FLAG_USE_CODEC_TS;

    while (!mStopPlayback)
    {

        while(!mStopPlayback && mStruct->buffer->size() <= 0)
        {
        }

        int stringLength = mStruct->buffer->at(0);
        if(mStopPlayback)
        {
            //return AVERROR_EOF;
            break;
        }
        mStruct->writeLock->lock();

        mStruct->buffer->remove(0, 1);

        mStruct->writeLock->unlock();


        qDebug() << "Stringlength: " << stringLength;
        while(!mStopPlayback && mStruct->buffer->size() <= stringLength)
        {
        }

        QByteArray sizeArray = QByteArray(mStruct->buffer->data(), stringLength);
        qDebug() << "sizearray: " << sizeArray;
        QString sizeString = QString(sizeArray);
        qDebug() << "sizestring: " << sizeString;
        int buffer_size = sizeString.toInt();

        while (!mStopPlayback && mStruct->buffer->size() <= (buffer_size + stringLength))
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


        qDebug() << "Before lock";
        if(mStopPlayback)
        {
            //return AVERROR_EOF;
            break;
        }
        mStruct->writeLock->lock();
        qDebug() << "After lock";
        mStruct->buffer->remove(0, stringLength);

        QByteArray tempBuffer = QByteArray(mStruct->buffer->data(), buffer_size);
        mStruct->buffer->remove(0, buffer_size);
        mStruct->writeLock->unlock();


        memcpy(mRecvbuf, tempBuffer.constData(), buffer_size);



        //int length = read(, recvbuf, 10e5);
        //int length = mStruct->buffer->size();

        int length = buffer_size;


        qDebug() << "Before length check" << length;
        int ret = parsePacket(parser, packet, length, videoDecoderCodecContext);
        if (ret < 0)
        {
            continue;
        }

        //Decode:
        error = avcodec_send_packet(videoDecoderCodecContext, packet);
        if (error == AVERROR_EOF || error == AVERROR(EOF))
        {
            qDebug() << "send packet sleep";
            /*int ms = 1000;
            struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
            nanosleep(&ts, NULL);*/
            continue;
        }
        else if(error == AVERROR(EAGAIN))
        {
            qDebug() << "Eagain on send packet!";

        }
        else if(error < 0)
        {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(error,errbuff,1000);
            qDebug() << "Failed udp input avcodec_send_packet: code "<<error<< " meaning: " << errbuff;
            //exit(1);
            av_packet_unref(packet);

            continue;
        }

        error = avcodec_receive_frame(videoDecoderCodecContext, frame);
        if (error == AVERROR(EAGAIN) || error == AVERROR_EOF){
            //skipped_frames++;
            qDebug() << "Skipped a Frame VideoPlaybackHandler";
            continue;
        }
        else if (error < 0)
        {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(error, errbuff, 1000);
            qDebug() << "Failed avcodec_receive_frame: code "<<error<< " meaning: " << errbuff;
            exit(1);
        }

        mImageHandler->readImage(videoDecoderCodecContext, frame, mIndex);

        av_frame_unref(frame);
        av_packet_unref(packet);

    }
    qDebug() << "Closing videoplaybackhandler";
    avformat_close_input(&inputFormatContext);
    avcodec_free_context(&videoDecoderCodecContext);

}

void VideoPlaybackHandler::decreaseIndex()
{
    mIndex--;
}

int VideoPlaybackHandler::parsePacket(AVCodecParserContext* parser, AVPacket* packet, const int& length, AVCodecContext* videoDecoderCodecContext)
{
    if (length == 0)
    {
        return -1;
    }
    //Creating temporary packet
    AVPacket * tempPacket = new AVPacket;
    av_init_packet(tempPacket);
    av_new_packet(tempPacket, length);
    memcpy(tempPacket->data, mRecvbuf, length);
    //tempPacket->pos = pos; //Do we need this?
    //pos += length; // ^^
    memset(mRecvbuf, 0, length);

    //Parsing temporary packet into pkt
    av_init_packet(packet);

    qDebug() << "Before Parse";
    av_parser_parse2(parser, videoDecoderCodecContext,
                     &(packet->data), &(packet->size),
                     tempPacket->data, tempPacket->size,
                     //AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0
                     tempPacket->pts, tempPacket->dts, tempPacket->pos
                     );

    packet->pts = parser->pts; //Might not need these
    packet->dts = parser->dts;
    packet->pos = parser->pos;
    qDebug() << "After Parse";

    //Set keyframe flag
    if (parser->key_frame == 1 || (parser->key_frame == -1 && parser->pict_type == AV_PICTURE_TYPE_I))
    {
        packet->flags |= AV_PKT_FLAG_KEY;
    }

    /*
    if (parser->key_frame == -1 && parser->pict_type == AV_PICTURE_TYPE_NONE && (packet->flags & AV_PKT_FLAG_KEY))
    {
        packet->flags |= AV_PKT_FLAG_KEY;
    }
    */
    packet->duration = 96000; //Same result as in av_read_frame()
    return 0;
}
