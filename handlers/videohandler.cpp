#include "videohandler.h"

#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */


VideoHandler::VideoHandler(QString cDeviceName, std::mutex* _writeLock,int64_t _time,
                           ImageHandler* imageHandler, UdpSocketHandler* _socketHandler,
                           int bufferSize, TcpSocketHandler* tcpSocketHandler, bool screenShare, QObject* parent): QObject(parent)
{
    mScreenCapture = screenShare;
    mTcpSocketHandler = tcpSocketHandler;

    connect(this, &VideoHandler::callWriteHeader, mTcpSocketHandler, &TcpSocketHandler::writeHeader);

    mBufferSize = bufferSize;
    mWriteToFile = false;
    mSocketHandler = _socketHandler;
    mTime = _time;
    //std::ofstream outfile("video.ismv", std::ostream::binary);
    //socketHandler = new SocketHandler();
    //socketHandler->initSocket();


    /*ScreenSharing stuff*/
    if(mScreenCapture)
    {
        mSource = "x11grab";
        this->mCameraDeviceName = buildScreenDeviceName();
    }
    else
    {
        this->mCameraDeviceName = cDeviceName;
        mSource = "v4l2";
    }

    //this->aDeviceName = aDeviceName;
    this->mImageHandler = imageHandler;
    mWriteLock = _writeLock;
    //ofmt_ctx = _ofmt_ctx;

    mStruct = new mSocketStruct;
    mStruct->udpSocket = mSocketHandler;
    mStruct->tcpSocket = tcpSocketHandler;
    mStruct->headerSent = false;
}



QString VideoHandler::buildScreenDeviceName()
{

    //Use system calls to find displayName and number for X11grab
    QString displayName = SystemCall::exec("xdpyinfo | grep 'name of display:'");
    QString displayNumber = SystemCall::exec("xdpyinfo | grep 'default screen number:'");
    qDebug() << "ScreenInfo system call: " << displayName;
    qDebug() << "ScreenInfo system call: " << displayNumber;
    for(int i = 0; i < displayName.length(); i++)
    {
        if(displayName.at(i).isDigit())
        {
            displayName = displayName.at(i);
            break;
        }
    }
    for(int i = 0; i < displayNumber.length(); i++)
    {
        if(displayNumber.at(i).isDigit())
        {
            displayNumber = displayNumber.at(i);
            break;
        }
    }

    QScreen* screen = QGuiApplication::primaryScreen();
    int a, b, c, d;
    screen->geometry().getCoords(&a, &b, &c, &d);

    QRect screenGeometry = screen->geometry();

    mScreenHeight = screenGeometry.height();
    mScreenWidth = screenGeometry.width();

    QString screenDeviceName = ":" + displayName + "." + displayNumber + "+" + QString::number(a) + ", " + QString::number(b) + "," +  QString::number(mScreenWidth) + "," + QString::number(mScreenHeight);
    qDebug() << "ScreenDeviceName: " << screenDeviceName;

    return screenDeviceName;
}

VideoHandler::~VideoHandler()
{
    delete mStruct;
}

int VideoHandler::init()
{
    mActive = false;
    ifmt_ctx = NULL;
    //ofmt_ctx = NULL;
    int ret;

    //Find input video formats
    AVInputFormat* videoInputFormat = av_find_input_format(mSource);

    if(videoInputFormat == NULL)
    {
        qDebug() << "Not found videoFormat\n";
        return -1;
    }

    AVDictionary* screenOpt = NULL;
    //std::string videoSize = QStringLiteral("%1x%2").arg(mScreenWidth).arg(mScreenHeight).toStdString();

    //std::string framerate = QString{}.setNum(1).toStdString();
    //av_dict_set(&screenOpt, "framerate", framerate.c_str(), 0);
    if(mScreenCapture)
    {
        QString videoSize = QString::number(mScreenWidth) + "x" + QString::number(mScreenHeight);
        std::string framerate = QString{}.setNum(30).toStdString();
        av_dict_set(&screenOpt, "framerate", framerate.c_str(), 0);
        av_dict_set(&screenOpt, "video_size", videoSize.toUtf8().data(), 0);
        av_dict_set(&screenOpt, "probesize", "800000000", 0);
    }

    //Open VideoInput

    if (avformat_open_input(&ifmt_ctx, mCameraDeviceName.toUtf8().data(), videoInputFormat, &screenOpt) < 0)
    {
        fprintf(stderr, "Could not open input file '%s'", mCameraDeviceName.toUtf8().data());
        return -1;
    }

    av_dict_free(&screenOpt);

    //Get stream information
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
    {
        fprintf(stderr, "Failed to retrieve input stream information");
        return -1;
    }
    //Print stream information
    qDebug() << "Dumping video input";
    av_dump_format(ifmt_ctx, 0, NULL, 0);


    ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, "ismv", NULL);

    if (ret < 0)
    {
        fprintf(stderr, "Could not alloc output context with file '%s'", mFileName);
        exit(1);
    }

    //av_opt_set(ofmt_ctx, "preset", "slow", 0);
    //av_opt_set(ofmt_ctx, "crf", "40", 0);

    //Set Output codecs from guess
    outputVideoCodec = avcodec_find_encoder(ofmt_ctx->oformat->video_codec);

    // av_opt_set(outputVideoCodec, "preset", "slow", 0);
    //` av_opt_set(outputVideoCodec, "crf", "28", 0);

    //Allocate CodecContext for outputstreams
    mOutputVideoCodecContext = avcodec_alloc_context3(outputVideoCodec);

    //Loop gjennom inputstreams
    for (int i = 0; (unsigned int)i < ifmt_ctx->nb_streams; i++)
    {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream;

        //Hvis instream er Video
        //if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {

        qDebug() << "Input stream framerate: " << in_stream->r_frame_rate.num;
        qDebug() << "Input stream timebase: " << in_stream->time_base.num << "/" << in_stream->time_base.den;



        //Setter av inputcodec og codeccontext, så vi slipper bruke deprecated codec
        inputVideoCodec = avcodec_find_decoder((ifmt_ctx)->streams[i]->codecpar->codec_id);
        mInputVideoCodecContext = avcodec_alloc_context3(inputVideoCodec);
        avcodec_parameters_to_context(mInputVideoCodecContext, in_stream->codecpar);
        ret = avcodec_open2(mInputVideoCodecContext, inputVideoCodec, NULL);
        mInputVideoCodecContext->framerate = in_stream->r_frame_rate;
        mInputVideoCodecContext->time_base = in_stream->time_base;

        //Lager ny outputStream
        out_stream = avformat_new_stream(ofmt_ctx, outputVideoCodec);
        //Denne trenger vi egentlig ikke lenger
        mVideoStream = i;
        //Setter div parametere.
        //Denne krasher vanlig video også
        //outputVideoCodecContext->bit_rate = in_stream->codecpar->bit_rate;
        mOutputVideoCodecContext->width = in_stream->codecpar->width;
        mOutputVideoCodecContext->height = in_stream->codecpar->height;


        //HARDKODET WIDTH OG HEIGHT PGA at framerate osv hos v4l2 er bare piss!! Gjelder bare hos Kent
        //EDIT: gjelder antakelig alle etter skjermdeling ble lagt til.
        //qDebug() << in_stream->codecpar->width;
        //qDebug() << in_stream->codecpar->height;
        //outputVideoCodecContext->width = 960;
        //outputVideoCodecContext->height = 540;
        //mOutputVideoCodecContext->width = 800;
        //mOutputVideoCodecContext->height = 450;
        //outputVideoCodecContext->width = 640;
        //outputVideoCodecContext->height = 360;
        if(mScreenCapture)
        {
            mOutputVideoCodecContext->width = 1920;
            mOutputVideoCodecContext->height = 1080;

            //double ratio = in_stream->codecpar->width / in_stream->codecpar->height;

            //outputVideoCodecContext->height = outputVideoCodecContext->width / ratio;

            //outputVideoCodecContext->bit_rate = 80000000;
            //outputVideoCodecContext->framerate = (AVRational){10,1};
            //outputVideoCodecContext->width = in_stream->codecpar->width;
            //outputVideoCodecContext->height = in_stream->codecpar->height;
        }

        mOutputVideoCodecContext->pix_fmt = STREAM_PIX_FMT;
        mOutputVideoCodecContext->time_base = mInputVideoCodecContext->time_base;
        //outputVideoCodecContext->time_base = (AVRational){ 1, 10 };
        mOutputVideoCodecContext->max_b_frames = 0;
        //outputVideoCodecContext->framerate = inputVideoCodecContext->framerate;
        mOutputVideoCodecContext->gop_size = 1;
        //mOutputVideoCodecContext->flags |= 0x00080000;

        av_opt_set(mOutputVideoCodecContext->priv_data, "preset", "veryslow", 0);
        //av_opt_set(mOutputVideoCodecContext->priv_data, "crf", "36", 0);//0 is lossless, 53 is worst possible quality
        av_opt_set(mOutputVideoCodecContext->priv_data, "tune", "zerolatency", 0);//0 is lossless, 53 is worst possible quality
        //av_opt_set(outputVideoCodecContext->priv_data, "qmin", "15", 0);
        //av_opt_set(outputVideoCodecContext->priv_data, "qmax", "35", 0);

        //qDebug() << outputVideoCodecContext-


        //outputVideoCodecContext->level = FF_LEVEL_UNKNOWN;





        //Kopierer parametere inn i out_stream
        avcodec_parameters_from_context(out_stream->codecpar, mOutputVideoCodecContext);
        ret = avcodec_open2(mOutputVideoCodecContext, outputVideoCodec, NULL);

        out_stream->time_base = in_stream->time_base;

        //Sett convert context som brukes ved frame conversion senere.
        img_convert_ctx = sws_getContext(
                    in_stream->codecpar->width,
                    in_stream->codecpar->height,
                    //in_stream->codec->pix_fmt,
                    (AVPixelFormat)in_stream->codecpar->format,
                    mOutputVideoCodecContext->width,
                    mOutputVideoCodecContext->height,
                    mOutputVideoCodecContext->pix_fmt,
                    SWS_BICUBIC,
                    NULL, NULL, NULL);
        //previous_pts = in_stream->start_time;


        //}
        if (!out_stream)
        {
            fprintf(stderr, "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            return -1;
        }
        out_stream->codecpar->codec_tag = 0;



        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        {
            //out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            mOutputVideoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }
        //usikker på om denne må være før eller etter.
        ret = avcodec_parameters_to_context(mOutputVideoCodecContext, out_stream->codecpar);
        if (ret < 0)
        {
            qDebug() << "avcodec_parameters_to_context failed" << Q_FUNC_INFO;
            exit(-1);
        }

        /*
        if (ofmt_ctx->oformat->flags & AVFMT_FLAG_NONBLOCK)
        {
            qDebug() << "AAAAAAAAAAAAAAAAAAAAAAAAAAAA";
            out_stream->codec->flags |= AVFMT_FLAG_NONBLOCK;
            return AVERROR(EAGAIN);
        }
        out_stream->codec->flags |= AVFMT_FLAG_NONBLOCK;
        */
    }

    AVDictionary *options = NULL;

    if(mScreenCapture)
    {
        //QString videoSize = QString::number(mScreenWidth) + "x" + QString::number(mScreenHeight);
        //std::string framerate = QString{}.setNum(10).toStdString();
        //av_dict_set(&options, "framerate", framerate.c_str(), 0);
        //av_dict_set(&options, "video_size", videoSize.toUtf8().data(), 0);
    }

    int avio_buffer_size = mBufferSize;
    void *avio_buffer = av_malloc(avio_buffer_size);
    AVIOContext* custom_io = avio_alloc_context (
                (unsigned char*)avio_buffer, avio_buffer_size,
                1, (void*) mStruct,
                NULL, &custom_io_write, NULL);
    ofmt_ctx->pb = custom_io;
    av_dict_set(&options, "live", "1", 0);

    qDebug() << "Dumping video output";
    av_dump_format(ofmt_ctx, 0, NULL, 1);

    ret = avformat_write_header(ofmt_ctx, &options);
    if(ret < 0)
    {
        fprintf(stderr, "Could not open write header");
        exit(1);
    }


    mStruct->headerSent = true;



    return ret;
}

//static int64_t mPts = 0;

void VideoHandler::grabFrames()
{
    init();

    emit callWriteHeader();

    mActive = true;
    AVPacket* pkt = av_packet_alloc();
    AVStream* out_stream;
    AVPacket* outPacket = av_packet_alloc();

    pkt->size = 0;
    pkt->data = NULL;

    if(pkt == NULL)
    {
        qDebug() << "pkt = null\n";
        exit(1);
    }
    mVideoFrame = av_frame_alloc();
    mVideoFrame->data[0] = NULL;
    mVideoFrame->width = mInputVideoCodecContext->width;
    mVideoFrame->height = mInputVideoCodecContext->height;
    mVideoFrame->format = mInputVideoCodecContext->pix_fmt;

    mScaledFrame = av_frame_alloc();
    mScaledFrame->data[0] = NULL;
    mScaledFrame->width = mOutputVideoCodecContext->width;
    mScaledFrame->height = mOutputVideoCodecContext->height;
    mScaledFrame->format = mOutputVideoCodecContext->pix_fmt;

    int ret;
    while ((ret = av_read_frame(ifmt_ctx, pkt)) >= 0 && !mAbortGrabFrames)
    {

        //qDebug() << "Input codec framerate: " << inputVideoCodecContext->framerate.num;
        //qDebug() << "Input codec timebase: " << inputVideoCodecContext->time_base.num << "/" << inputVideoCodecContext->time_base.den;
        //pkt->pts = av_gettime();


        /*if(firstPacket)
            {
                start_pts = pkt->pts;
                start_dts = pkt->dts;
                firstPacket = false;
            }*/

        //pkt->pts = pkt->pts - start_pts;
        //int64_t dts = av_gettime();
        //dts = dts * inputVideoCodecContext->framerate.num;
        //pkt->dts = pkt->dts - dts - start_dts;



        //qDebug() << "kommer inn i videoStreamgreiene\n";
        if(ret < 0)
        {
            qDebug() << "Input Avcodec open failed: " << ret << "\n";
            exit(1);
        }
        //qDebug() << "Forbi avodec_open\n";
        ret = avcodec_send_packet(mInputVideoCodecContext, pkt);
        if(ret < 0)
        {
            qDebug() << "Send packet error";
            exit(1);
        }

        //qDebug() << "Forbi send packet\n";
        ret = avcodec_receive_frame(mInputVideoCodecContext, mVideoFrame);
        if(ret < 0)
        {
            qDebug() << "Recieve frame error";
            exit(1);
        }
        //If the input video format is not STREAM_PIX_FMT or if the input w/h does not match output w/h, do rescaling++
        if (mInputVideoCodecContext->pix_fmt != STREAM_PIX_FMT ||
                mOutputVideoCodecContext->width != mInputVideoCodecContext->width ||
                mOutputVideoCodecContext->height != mInputVideoCodecContext->height)
        {
            int num_bytes = av_image_get_buffer_size(mOutputVideoCodecContext->pix_fmt, mOutputVideoCodecContext->width,
                                                     mOutputVideoCodecContext->height, 1);

            uint8_t* frame2_buffer = (uint8_t *)av_malloc(num_bytes*sizeof(uint8_t));

            av_image_fill_arrays(mScaledFrame->data, mScaledFrame->linesize,
                                 frame2_buffer, mOutputVideoCodecContext->pix_fmt,
                                 mOutputVideoCodecContext->width, mOutputVideoCodecContext->height, 1);

            ret = sws_scale(img_convert_ctx, mVideoFrame->data,
                            mVideoFrame->linesize, 0,
                            mInputVideoCodecContext->height,
                            mScaledFrame->data, mScaledFrame->linesize);
            //qDebug() << "Etter swsScale\n";
            if(ret < 0)
            {
                qDebug() << "Error with scale " << ret <<"\n";
                exit(1);
            }
            if(mIsFirstPacket)
            {
                mPts = mTime;
                mIsFirstPacket = false;
            }

            if (mScaledFrame)
            {
                mScaledFrame->pts = mPts;
                mPts += ifmt_ctx->streams[0]->time_base.den/ifmt_ctx->streams[0]->r_frame_rate.num;
            }
            mImageHandler->readImage(mOutputVideoCodecContext, mScaledFrame, std::numeric_limits<uint8_t>::max());
            ret = avcodec_send_frame(mOutputVideoCodecContext, mScaledFrame);
            if(ret < 0)
            {
                qDebug() << "Error with send frame " << ret <<"\n";

                exit(1);
            }

            av_free(frame2_buffer); //Viktig! Ellers skjer det memory leaks.
        }
        else
        {
            if(mIsFirstPacket)
            {
                mPts = mTime;
                mIsFirstPacket = false;
            }

            if (mVideoFrame)
            {
                mVideoFrame->pts = mPts;
                mPts += ifmt_ctx->streams[0]->time_base.den/ifmt_ctx->streams[0]->r_frame_rate.num;
            }
            mImageHandler->readImage(mOutputVideoCodecContext, mVideoFrame, std::numeric_limits<uint8_t>::max());
            ret = avcodec_send_frame(mOutputVideoCodecContext, mVideoFrame);
            //av_frame_free(&videoFrame);
            if(ret < 0)
            {
                qDebug() << "Error with send frame " << ret <<"\n";
                exit(1);
            }
        }
        //qDebug() << "Etter sendFrame\n";


        outPacket->data = NULL;
        outPacket->size = 0;
        if(ret < 0)
        {
            qDebug() << "Output Avcodec open failed: " << ret << "\n";
        }

        ret = avcodec_receive_packet(mOutputVideoCodecContext, outPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            mNumberOfSkippedFrames++;
            //qDebug() << "Skipped a Frame";
            continue;
        }
        else if (ret < 0)
        {
            fprintf(stderr, "Error with receive packet\n");
            exit(1);
        }
        else
        {
            /*if(firstPacket)
            {
                start_pts = outPacket->pts;
                start_dts = outPacket->dts;
                firstPacket = false;
            }

            outPacket->pts = outPacket->pts - start_pts;
            outPacket->dts = outPacket->dts - start_dts;
*/
            //outPacket->dts = av_gettime();
            //outPacket->pts = av_gettime();



            //qDebug() << "ready for write";
            mNumberOfSkippedFrames = 0;


            //in_stream  = ifmt_ctx->streams[pkt->stream_index];
            out_stream = ofmt_ctx->streams[pkt->stream_index];
            //out_stream->avg_frame_rate = (AVRational){60, 1};
            //out_stream->time_base = (AVRational){1, 60};

            //out_stream->codec->gop_size = 30;
            //out_stream->codec->max_b_frames = 1;

            //out_stream->codec->framerate = AVRational{30,1};
            //out_stream->time_base = AVRational{1, 30};
            AVRational encoderTimebase = mOutputVideoCodecContext->time_base;//{1, 30};
            AVRational muxerTimebase = out_stream->time_base;
            //                qDebug() << "**********VIDEO*****************";
            //                qDebug() << "Outpacket pts: " << outPacket->pts;
            //                qDebug() << "Outpacket dts: " << outPacket->dts;
            //                qDebug() << outPacket->stream_index;

            outPacket->pts = av_rescale_q_rnd(outPacket->pts, encoderTimebase, muxerTimebase, (AVRounding) (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            outPacket->dts = av_rescale_q_rnd(outPacket->dts, encoderTimebase, muxerTimebase, (AVRounding) (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            outPacket->duration = av_rescale_q(outPacket->duration, encoderTimebase, muxerTimebase);
            outPacket->pos = -1;

            //                qDebug() << "Outpacket pts: " << outPacket->pts;
            //                qDebug() << "Outpacket dts: " << outPacket->dts;


            mWriteLock->lock();
            //qDebug() << "Writing Video Packet";
            int ret = av_interleaved_write_frame(ofmt_ctx, outPacket);
            mWriteLock->unlock();


            //av_freep(&scaledFrame->data[0]);
            //av_frame_unref(scaledFrame);
            //av_freep(&videoFrame->data[0]);
            //av_frame_unref(videoFrame);


            //qDebug() << "Wrote video packet ret = " << ret;
            //int ret = av_write_frame(ofmt_ctx, outPacket);

            //int ret = av_write_frame(ofmt_ctx, pkt);
            if (ret < 0)
            {
                qDebug() << "Error muxing packet" << Q_FUNC_INFO;
                //break;
            }
            av_packet_unref(pkt);
            av_packet_unref(outPacket);
            //av_packet_free(&outPacket);

            //av_packet_free(&pkt);
            //av_packet_free(&outPacket);
        }
        av_packet_unref(pkt);
        av_packet_unref(outPacket);

        //static int count = 0;
        //qDebug() << count << "/" << numberOfFrames;
        //if(count > numberOfFrames) break;
        //count++;
    }
    /*av_free(pkt);
    av_free(outPacket);
    av_free(mScaledFrame);
    av_free(mVideoFrame);*/
    av_packet_free(&pkt);
    av_packet_free(&outPacket);

    av_frame_free(&mScaledFrame);
    av_frame_free(&mVideoFrame);
    close();

    mImageHandler->setPeerVideoAsDisabled(std::numeric_limits<uint8_t>::max());


    /*
    qDebug() << "About to write trailer from video";
    writeLock->lock();
    //av_write_trailer(ofmt_ctx);
    writeLock->unlock();
    //outfile.close();
    avformat_close_input(&ifmt_ctx);
    // close output
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        avio_close(ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF)
    {
        //return -1;
        //fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
    }
    qDebug() << "Ferdig med grabFrames!!!\n";
    */
}

void VideoHandler::close()
{
    avcodec_free_context(&mOutputVideoCodecContext);
    avcodec_free_context(&mInputVideoCodecContext);
    avformat_close_input(&ifmt_ctx);
    if (ofmt_ctx->pb)
    {
           av_freep(&ofmt_ctx->pb->buffer);
    }
    avio_context_free(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    sws_freeContext(img_convert_ctx);
    //av_free(avio_buffer);
    mActive = false;
}

bool VideoHandler::isActive() const
{
    return mActive;
}

int VideoHandler::custom_io_write(void* opaque, uint8_t *buffer, int buffer_size)
{
    //qDebug() << "Inne i custom io write";


    mSocketStruct *s = reinterpret_cast<mSocketStruct*>(opaque);



    //SocketHandler* socketHandler = reinterpret_cast<SocketHandler*>(opaque);
    char *cptr = reinterpret_cast<char*>(const_cast<uint8_t*>(buffer));

    QByteArray send;
    send = QByteArray(reinterpret_cast<char*>(cptr), buffer_size);



    if(!s->headerSent)
    {
        qDebug() << "INNE I HEADERSEND";

        QString stringLength = QString::number(send.size());
        send.prepend(stringLength.toUtf8().data());
        send.prepend(stringLength.size());
        s->tcpSocket->appendToHeader(send);
        return 0;
    }
    else
    {
        //Prepends the video header byte needed by socketHandler

        QString stringLength = QString::number(send.size());
        send.prepend(stringLength.toUtf8().data());
        send.prepend(stringLength.size());



        send.prepend(int(1));
        return s->udpSocket->sendDatagram(send);

    }

}

void VideoHandler::toggleGrabFrames(bool a)
{
    mAbortGrabFrames = !a;
}










