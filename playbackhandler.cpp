#include "playbackhandler.h"

PlaybackHandler::PlaybackHandler(QObject *parent) : QMediaPlayer(parent)
{
    //udpSocket = new QUdpSocket();
}

void PlaybackHandler::setVideoSurface(QAbstractVideoSurface* surface)
{
    qDebug() << "Changing surface";
    m_surface = surface;
    setVideoOutput(m_surface);
}

QAbstractVideoSurface* PlaybackHandler::getVideoSurface()
{
    return m_surface;
}

void PlaybackHandler::playFromFile(const QString &strFile)
{
    QMediaPlayer::setMedia(QUrl::fromLocalFile(strFile));
    //QMediaPlayer::setVideoOutput(m_surface);
    QMediaPlayer::play();
    qDebug() << "Tried to play";
}

void PlaybackHandler::getStream()
{

}

/*
int PlaybackHandler::read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    //auto	fil = static_cast<QUdpSocket*>(opaque);
        //QMutexLocker	l(&fil->mutex);
        //qDebug() << buf_size << fil->buffer.length();

        qDebug() << buf_size << buffer.length();

        if (buffer.isEmpty() && !udpSocket->hasPendingDatagrams()) udpSocket->waitForReadyRead();
        while (udpSocket->hasPendingDatagrams()) {
            if (buffer.length() > 1024*500) {
                buffer = buffer.right(1024*500);
                buffer.clear();
            }
            buffer.append(udpSocket->receiveDatagram().data());
        }

        buf_size = buffer.length() > buf_size ? buf_size : buffer.length();
        memcpy(buf, buffer.constData(), buf_size);
        buffer.remove(0, buf_size);

        return buf_size;
}

int PlaybackHandler::start()
{
    QtConcurrent::run([this]()
    {
        udpSocket = new QUdpSocket;
        udpSocket->bind(15004);

        AVFormatContext *fmt_ctx = nullptr;
        AVIOContext *avio_ctx = nullptr;
        uint8_t *buffer = nullptr, *avio_ctx_buffer = nullptr;
        size_t buffer_size = 0, avio_ctx_buffer_size = 4096;

        int ret = 0;
        fmt_ctx = avformat_alloc_context();
        Q_ASSERT(fmt_ctx);

        avio_ctx_buffer = reinterpret_cast<uint8_t*>(av_malloc(avio_ctx_buffer_size));
        Q_ASSERT(avio_ctx_buffer);
        avio_ctx = avio_alloc_context(avio_ctx_buffer, static_cast<int>(avio_ctx_buffer_size), 0, /*&bdudpSocket, read_packet(), nullptr, nullptr);
        Q_ASSERT(avio_ctx);

        fmt_ctx->pb = avio_ctx;
        ret = avformat_open_input(&fmt_ctx, nullptr, nullptr, nullptr);
        Q_ASSERT(ret >= 0);
        ret = avformat_find_stream_info(fmt_ctx, nullptr);
        Q_ASSERT(ret >= 0);

        AVStream	*video_stream = nullptr;
        for (uint i=0; i<fmt_ctx->nb_streams; ++i) {
            auto	st = fmt_ctx->streams[i];
            qDebug() << st->id << st->index << st->start_time << st->duration << st->codecpar->codec_type;
            if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) video_stream = st;
        }
        Q_ASSERT(video_stream);


        AVCodecParameters	*codecpar = video_stream->codecpar;
        AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
        AVCodecContext *codec_context = avcodec_alloc_context3(codec);
        int err = avcodec_open2(codec_context, codec, nullptr);
        Q_ASSERT(err>=0);
        qDebug() << codec->name << codec->id;
        qDebug() << codecpar->width << codecpar->height << codecpar->format << codec_context->pix_fmt;




        AVFrame	*frameRGB = av_frame_alloc();
        frameRGB->format = AV_PIX_FMT_RGB24;
        frameRGB->width = codecpar->width;
        frameRGB->height = codecpar->height;
        err = av_frame_get_buffer(frameRGB, 0);
        Q_ASSERT(err == 0);
        ///
        SwsContext *imgConvertCtx = nullptr;

        AVFrame* frame = av_frame_alloc();
        AVPacket packet;

        while (av_read_frame(fmt_ctx, &packet) >= 0) {
            avcodec_send_packet(codec_context, &packet);
            err = avcodec_receive_frame(codec_context, frame);
            if (err == 0) {
                //qDebug() << frame->height << frame->width << codec_context->pix_fmt;

                imgConvertCtx = sws_getCachedContext(imgConvertCtx,
                                                     codecpar->width, codecpar->height, static_cast<AVPixelFormat>(codecpar->format),
                                                     frameRGB->width, frameRGB->height,
                                                     AV_PIX_FMT_RGB24, SWS_BICUBIC, nullptr, nullptr, nullptr);
                Q_ASSERT(imgConvertCtx);

                //conversion frame to frameRGB
                sws_scale(imgConvertCtx, frame->data, frame->linesize, 0, codec_context->height, frameRGB->data, frameRGB->linesize);

                //setting QImage from frameRGB
                QImage image(frameRGB->data[0],
                        frameRGB->width,
                        frameRGB->height,
                        frameRGB->linesize[0],
                        QImage::Format_RGB888);

                emit imageUpdated(image, clock());
            }

            av_frame_unref(frame);
            av_packet_unref(&packet);
        }
        if (imgConvertCtx) sws_freeContext(imgConvertCtx);





        //end:
        avformat_close_input(&fmt_ctx);
        /* note: the internal buffer could have changed, and be != avio_ctx_buffer
        if (avio_ctx) {
            av_freep(&avio_ctx->buffer);
            av_freep(&avio_ctx);
        }
        av_file_unmap(buffer, buffer_size);
        if (ret < 0) {
            qWarning() << ret;
            return 1;
        }
        return 0;
    });
}
*/

