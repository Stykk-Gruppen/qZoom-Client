#include "imagehandler.h"

ImageHandler::ImageHandler() : QQuickImageProvider(QQuickImageProvider::Image)
{
    this->no_image = QImage("0.png");
    this->blockSignals(false);
    this->mFramesFinished = 1;
}

QImage ImageHandler::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
//Jeg tror id må være her selvom den ikke blir brukt. Det er QML som kaller funksjonen på en usynlig måte.
{
    QImage result = this->image;

    if(result.isNull())
    {
        result = this->no_image;
        qDebug() << "Result is null";
    }

    if(size)
    {
        *size = result.size();
    }

    if(requestedSize.width() > 0 && requestedSize.height() > 0)
    {
        result = result.scaled(requestedSize.width(), requestedSize.height(), Qt::KeepAspectRatio);
    }

    return result;
}

void ImageHandler::updateImage(const QImage &image)
{
    if(this->image != image)
    {
        this->image = image;
        emit imageChanged();
    }
}

void ImageHandler::veryFunStianLoop()
{
    QtConcurrent::run([this]()
    {
        int ms = 41; //1000/24
        int i = 1;
        QString overhead = "";
        while (true)
        {
            if (i < 100)
            {
                overhead = "0";
                if (i < 10)
                {
                    overhead = "00";
                }
            }
            else
            {
                overhead = "";
            }
            QString str = "/home/stian/Downloads/testtttt/" + overhead + QString::number(i) + ".jpg";
            QImage test = QImage(str);
            updateImage(test);
            if (i == 382)
            {
                i = 1;
            }
            i++;

            struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
            nanosleep(&ts, NULL);
        }
    });
}

void ImageHandler::readLocalImage(AVCodec* codec, AVCodecContext* codecContext, AVFrame* frame)
{
    const int width = 400;
    const int height = 400;
    //QImage img( width, height, QImage::Format_RGB888 );

    AVFrame	*frameRGB = av_frame_alloc();
    SwsContext *imgConvertCtx = nullptr;

    avpicture_alloc( ( AVPicture *) frameRGB, AV_PIX_FMT_RGB24, width, height);
    imgConvertCtx = sws_getContext( codecContext->width, codecContext->height,
                                     codecContext->pix_fmt, width, height, AV_PIX_FMT_RGB24,
                                     SWS_BICUBIC, NULL, NULL, NULL);
    if( 1 == mFramesFinished && nullptr != imgConvertCtx )
    {
    //conversion frame to frameRGB
    sws_scale(imgConvertCtx, frame->data, frame->linesize, 0, codecContext->height, frameRGB->data, frameRGB->linesize);
    //setting QImage from frameRGB
    /*
    for( int y = 0; y < height; ++y )
       memcpy( img.scanLine(y), frameRGB->data[0]+y * frameRGB->linesize[0], frameRGB->linesize[0] );
    } */
    qDebug() << frame->data;
    qDebug() << frameRGB->data;

    QImage image(frameRGB->data[0],
            frameRGB->width,
            frameRGB->height,
            frameRGB->linesize[0],
            QImage::Format_RGB888);

    emit updateImage(image);
    }
}

/*
void ImageHandler::readPacket(uint8_t *buffer, int buffer_size)
{
    QtConcurrent::run([this]()
    {
        AVFormatContext *fmt_ctx = nullptr;
        AVIOContext *avio_ctx = nullptr;
        uint8_t *avio_ctx_buffer = nullptr;
        size_t avio_ctx_buffer_size = 4096;

        int ret = 0;
        fmt_ctx = avformat_alloc_context();
        Q_ASSERT(fmt_ctx);

        avio_ctx_buffer = reinterpret_cast<uint8_t*>(av_malloc(avio_ctx_buffer_size));
        Q_ASSERT(avio_ctx_buffer);
        //avio_ctx = avio_alloc_context(avio_ctx_buffer, static_cast<int>(avio_ctx_buffer_size), 0, &bdudpSocket, read_packet(), nullptr, nullptr);
        //avio_ctx = avio_alloc_context()
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

                emit updateImage(image);
            }

            av_frame_unref(frame);
            av_packet_unref(&packet);
        }
        if (imgConvertCtx) sws_freeContext(imgConvertCtx);

        //end:
        avformat_close_input(&fmt_ctx);
    });
}
*/
