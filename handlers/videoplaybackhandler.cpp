#include "videoplaybackhandler.h"
#define 	CODEC_CAP_TRUNCATED   0x0008
#define     CODEC_FLAG_TRUNCATED 0x00010000

/**
 * @brief VideoPlaybackHandler::VideoPlaybackHandler
 * @param _writeLock
 * @param buffer
 * @param bufferSize
 * @param _imageHandler
 * @param index
 */
VideoPlaybackHandler::VideoPlaybackHandler(std::mutex* _writeLock, QByteArray* buffer,
                                           size_t bufferSize, ImageHandler* _imageHandler,
                                           int index) : Playback(_writeLock, buffer, bufferSize, _imageHandler, index)
{

}

/**
 * @brief VideoPlaybackHandler::~VideoPlaybackHandler
 */
VideoPlaybackHandler::~VideoPlaybackHandler()
{

}

/**
 * @brief VideoPlaybackHandler::start
 */
void VideoPlaybackHandler::start()
{
    mStopPlayback = false;
    int error = 0;
    AVFormatContext *inputFormatContext = avformat_alloc_context();
    Q_ASSERT(inputFormatContext);

    uint8_t *avioContextBuffer = reinterpret_cast<uint8_t*>(av_malloc(mBufferSize));
    Q_ASSERT(avioContextBuffer);

    AVIOContext *avioContext = avio_alloc_context(avioContextBuffer, static_cast<int>(mBufferSize), 0, mStruct, &customReadPacket, nullptr, nullptr);
    Q_ASSERT(avioContext);

    inputFormatContext->pb = avioContext;

    error = avformat_open_input(&inputFormatContext, NULL, NULL, NULL);
    qDebug() << "HEADER RECEIVED" << Q_FUNC_INFO;
    if(mStopPlayback)
    {
        return;
    }
    if(error < 0)
    {
        char* errbuff = (char *)malloc((1000)*sizeof(char));
        av_strerror(error,errbuff,1000);
        qDebug() << "AVformat open input UDP stream failed" << errbuff;
        exit(1);
    }

    qDebug() << "Dumping videoplayback format";
    av_dump_format(inputFormatContext, 0, NULL, 0);

    AVStream* video_stream = nullptr;
    for (uint i = 0; i < inputFormatContext->nb_streams; ++i)
    {
        AVStream* st = inputFormatContext->streams[i];
        if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream = st;
        }
    }

    if (!video_stream)
    {
        qDebug() << "Not a videostream!";
        return;
    }

    AVCodecParameters	*videoStreamCodecParameters = video_stream->codecpar;
    AVCodec* videoDecoderCodec = avcodec_find_decoder(videoStreamCodecParameters->codec_id);
    AVCodecContext *videoDecoderCodecContext;

    videoDecoderCodecContext = avcodec_alloc_context3(videoDecoderCodec);
    videoDecoderCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

    if (videoDecoderCodec->capabilities & CODEC_CAP_TRUNCATED)
    {
        videoDecoderCodecContext->flags |= CODEC_FLAG_TRUNCATED;
    }
    videoDecoderCodecContext->thread_type  = FF_THREAD_SLICE;
    videoDecoderCodecContext->thread_count = 2;

    error = avcodec_parameters_to_context(videoDecoderCodecContext, video_stream->codecpar);

    error = avcodec_open2(videoDecoderCodecContext, videoDecoderCodec, nullptr);
    Q_ASSERT(error >= 0);
    qDebug() << "codec name: " << videoDecoderCodec->name<< " codec id " << videoDecoderCodec->id;
    qDebug() << "codecpar width" << videoStreamCodecParameters->width <<" h: "<< videoStreamCodecParameters->height << " format: "<< videoStreamCodecParameters->format<< " pix fmt: " << videoDecoderCodecContext->pix_fmt;

    AVFrame* frame = av_frame_alloc();
    AVPacket* packet = av_packet_alloc();

    memset(mRecvbuf, 0, 10e5);

    AVCodecParserContext * parser = av_parser_init(AV_CODEC_ID_H264);
    parser->flags |= PARSER_FLAG_COMPLETE_FRAMES;
    parser->flags |= PARSER_FLAG_USE_CODEC_TS;

    while (!mStopPlayback)
    {
        while(!mStopPlayback && mStruct->buffer->size() <= 0)
        {
        }

        int stringLength = mStruct->buffer->at(0);
        if (mStopPlayback)
        {
            break;
        }
        mStruct->writeLock->lock();

        mStruct->buffer->remove(0, 1);

        mStruct->writeLock->unlock();

        while(!mStopPlayback && mStruct->buffer->size() <= stringLength)
        {
        }

        QByteArray sizeArray = QByteArray(mStruct->buffer->data(), stringLength);
        QString sizeString = QString(sizeArray);
        int buffer_size = sizeString.toInt();

        while (!mStopPlayback && mStruct->buffer->size() <= (buffer_size + stringLength))
        {
            if (*mStruct->stopPlayback)
            {
                break;
            }
        }

        if (mStopPlayback)
        {
            break;
        }
        mStruct->writeLock->lock();
        mStruct->buffer->remove(0, stringLength);

        QByteArray tempBuffer = QByteArray(mStruct->buffer->data(), buffer_size);
        mStruct->buffer->remove(0, buffer_size);
        mStruct->writeLock->unlock();


        memcpy(mRecvbuf, tempBuffer.constData(), buffer_size);

        int length = buffer_size;

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
            av_packet_unref(packet);

            continue;
        }

        error = avcodec_receive_frame(videoDecoderCodecContext, frame);
        if (error == AVERROR(EAGAIN) || error == AVERROR_EOF){
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
    avformat_close_input(&inputFormatContext);
    avcodec_free_context(&videoDecoderCodecContext);
}

/**
 * @brief VideoPlaybackHandler::decreaseIndex
 */
void VideoPlaybackHandler::decreaseIndex()
{
    mIndex--;
}

/**
 * @brief VideoPlaybackHandler::parsePacket
 * @param parser
 * @param packet
 * @param length
 * @param videoDecoderCodecContext
 * @return
 */
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
    memset(mRecvbuf, 0, length);

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
    if (parser->key_frame == 1 || (parser->key_frame == -1 && parser->pict_type == AV_PICTURE_TYPE_I))
    {
        packet->flags |= AV_PKT_FLAG_KEY;
    }

    packet->duration = 96000;
    return 0;
}
