#include "videohandler.h"
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */


VideoHandler::VideoHandler(QString cDeviceName, std::mutex* _writeLock,int64_t _time,
                           ImageHandler* imageHandler, SocketHandler* _socketHandler,
                           int bufferSize, TcpSocketHandler* tcpSocketHandler, QObject* parent): QObject(parent)
{
    mBufferSize = bufferSize;
    writeToFile = false;
    socketHandler = _socketHandler;
    time = _time;
    //std::ofstream outfile("video.ismv", std::ostream::binary);
    //socketHandler = new SocketHandler();
    //socketHandler->initSocket();
    this->cDeviceName = cDeviceName;
    //this->aDeviceName = aDeviceName;
    this->imageHandler = imageHandler;
    writeLock = _writeLock;
    //ofmt_ctx = _ofmt_ctx;

    mStruct = new mSocketStruct;
    mStruct->udpSocket = socketHandler;
    mStruct->tcpSocket = tcpSocketHandler;
    mStruct->headerSent = false;
}

int VideoHandler::init()
{
    //Registrer div ting
    av_register_all();
    avcodec_register_all();
    avdevice_register_all();
    ifmt_ctx = NULL;
    //ofmt_ctx = NULL;
    int ret;

    //Find input video formats
    AVInputFormat* videoInputFormat = av_find_input_format("v4l2");
    if(videoInputFormat == NULL)
    {
        qDebug() << "Not found videoFormat\n";
        return -1;
    }
    //Open VideoInput
    if (avformat_open_input(&ifmt_ctx, cDeviceName.toUtf8().data(), videoInputFormat, NULL) < 0)
    {
        fprintf(stderr, "Could not open input file '%s'", cDeviceName.toUtf8().data());
        return -1;
    }


    //Get stream information
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0)
    {
        fprintf(stderr, "Failed to retrieve input stream information");
        return -1;
    }
    //Print stream information
    qDebug() << "Dumping video input";
    av_dump_format(ifmt_ctx, 0, NULL, 0);


    ret = avformat_alloc_output_context2(&ofmt_ctx, NULL,"ismv", NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not alloc output context with file '%s'", filename);
        exit(1);
    }
    //Set Output codecs from guess
    outputVideoCodec = avcodec_find_encoder(ofmt_ctx->oformat->video_codec);



    //Allocate CodecContext for outputstreams
    outputVideoCodecContext = avcodec_alloc_context3(outputVideoCodec);

    //Loop gjennom inputstreams
    for (int i = 0; (unsigned int)i < ifmt_ctx->nb_streams; i++)
    {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream;

        //Hvis instream er Video
        if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {

            qDebug() << "Input stream framerate: " << in_stream->r_frame_rate.num;
            qDebug() << "Input stream timebase: " << in_stream->time_base.num << "/" << in_stream->time_base.den;



            //Setter av inputcodec og codeccontext, så vi slipper bruke deprecated codec
            inputVideoCodec = avcodec_find_decoder((ifmt_ctx)->streams[i]->codecpar->codec_id);
            inputVideoCodecContext = avcodec_alloc_context3(inputVideoCodec);
            avcodec_parameters_to_context(inputVideoCodecContext, in_stream->codecpar);
            ret = avcodec_open2(inputVideoCodecContext, inputVideoCodec, NULL);
            inputVideoCodecContext->framerate = in_stream->r_frame_rate;
            inputVideoCodecContext->time_base = in_stream->time_base;

            //Lager ny outputStream
            out_stream = avformat_new_stream(ofmt_ctx, outputVideoCodec);
            //Denne trenger vi egentlig ikke lenger
            videoStream = i;
            //Setter div parametere.
           // outputVideoCodecContext->bit_rate = 1000;//in_stream->codecpar->bit_rate;
            outputVideoCodecContext->width = in_stream->codecpar->width;
            outputVideoCodecContext->height = in_stream->codecpar->height;
            //outputVideoCodecContext->width = 160;
            //outputVideoCodecContext->height = 120;
            outputVideoCodecContext->pix_fmt = STREAM_PIX_FMT;
            outputVideoCodecContext->time_base = inputVideoCodecContext->time_base;
            //outputVideoCodecContext->time_base = (AVRational){ 1, 10 };
            outputVideoCodecContext->max_b_frames = 2;
            //outputVideoCodecContext->framerate = inputVideoCodecContext->framerate;
            outputVideoCodecContext->gop_size = 0;

            av_opt_set(outputVideoCodecContext, "preset", "slow", 0);
            av_opt_set(outputVideoCodecContext, "crf", "22", 0);
            //outputVideoCodecContext->level = FF_LEVEL_UNKNOWN;


            //Kopierer parametere inn i out_stream
            avcodec_parameters_from_context(out_stream->codecpar, outputVideoCodecContext);
            ret = avcodec_open2(outputVideoCodecContext, outputVideoCodec, NULL);

            out_stream->time_base = in_stream->time_base;

            //Sett convert context som brukes ved frame conversion senere.
            img_convert_ctx = sws_getContext(
                        in_stream->codecpar->width,
                        in_stream->codecpar->height,
                        //in_stream->codec->pix_fmt,
                        (AVPixelFormat)in_stream->codecpar->format,
                        outputVideoCodecContext->width,
                        outputVideoCodecContext->height,
                        outputVideoCodecContext->pix_fmt,
                        SWS_BICUBIC,
                        NULL, NULL, NULL);
            //previous_pts = in_stream->start_time;


        }
        if (!out_stream)
        {
            fprintf(stderr, "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            return -1;
        }
        out_stream->codecpar->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        {
            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }
    }

    AVDictionary *options = NULL;
    int avio_buffer_size = mBufferSize;
    void* avio_buffer = av_malloc(avio_buffer_size);
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

    //if(!mStruct->headerSent)
    {
        mStruct->headerSent = true;
        mStruct->tcpSocket->writeHeader();
        qDebug() << "After tcp writeHeader";
    }

}

static int64_t pts = 0;

void VideoHandler::grabFrames() {

    AVPacket* pkt = av_packet_alloc();
    AVStream *in_stream, *out_stream;
    AVPacket* outPacket = av_packet_alloc();

    pkt->size = 0;
    pkt->data = NULL;

    if(pkt == NULL)
    {
        qDebug() << "pkt = null\n";
        exit(1);
    }
    videoFrame = av_frame_alloc();
    videoFrame->data[0] = NULL;
    videoFrame->width = inputVideoCodecContext->width;
    videoFrame->height = inputVideoCodecContext->height;
    videoFrame->format = inputVideoCodecContext->pix_fmt;

    scaledFrame = av_frame_alloc();
    scaledFrame->data[0] = NULL;
    scaledFrame->width = outputVideoCodecContext->width;
    scaledFrame->height = outputVideoCodecContext->height;
    scaledFrame->format = outputVideoCodecContext->pix_fmt;

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
        ret = avcodec_send_packet(inputVideoCodecContext, pkt);
        if(ret < 0)
        {
            qDebug() << "Send packet error";
            exit(1);
        }

        //qDebug() << "Forbi send packet\n";
        ret = avcodec_receive_frame(inputVideoCodecContext, videoFrame);
        if(ret < 0)
        {
            qDebug() << "Recieve frame error";
            exit(1);
        }

        if (inputVideoCodecContext->pix_fmt != STREAM_PIX_FMT)
        {
            int num_bytes = av_image_get_buffer_size(outputVideoCodecContext->pix_fmt, outputVideoCodecContext->width,
                                                     outputVideoCodecContext->height, 1);

            uint8_t* frame2_buffer = (uint8_t *)av_malloc(num_bytes*sizeof(uint8_t));

            av_image_fill_arrays(scaledFrame->data, scaledFrame->linesize,
                                 frame2_buffer, outputVideoCodecContext->pix_fmt,
                                 outputVideoCodecContext->width, outputVideoCodecContext->height, 1);

            ret = sws_scale(img_convert_ctx, videoFrame->data,
                            videoFrame->linesize, 0,
                            inputVideoCodecContext->height,
                            scaledFrame->data, scaledFrame->linesize);
            //qDebug() << "Etter swsScale\n";
            if(ret < 0)
            {
                qDebug() << "Error with scale " << ret <<"\n";
                exit(1);
            }
            if(firstPacket)
            {
                pts = time;
                firstPacket = false;
            }

            if (scaledFrame)
            {
                scaledFrame->pts = pts;
               pts += ifmt_ctx->streams[0]->time_base.den/ifmt_ctx->streams[0]->r_frame_rate.num;
            }
            imageHandler->readImage(outputVideoCodecContext, scaledFrame, 0);
            ret = avcodec_send_frame(outputVideoCodecContext, scaledFrame);
            if(ret < 0)
            {
                qDebug() << "Error with send frame " << ret <<"\n";

                exit(1);
            }

            av_free(frame2_buffer); //Viktig! Ellers skjer det memory leaks.
        }
        else
        {
            if(firstPacket)
            {
                pts = time;
                firstPacket = false;
            }

            if (videoFrame)
            {
                videoFrame->pts = pts;
                pts += ifmt_ctx->streams[0]->time_base.den/ifmt_ctx->streams[0]->r_frame_rate.num;
            }
            imageHandler->readImage(outputVideoCodecContext, videoFrame, 0);
            ret = avcodec_send_frame(outputVideoCodecContext, videoFrame);
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

        ret = avcodec_receive_packet(outputVideoCodecContext, outPacket);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            skipped_frames++;
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
            skipped_frames = 0;


            in_stream  = ifmt_ctx->streams[pkt->stream_index];
            out_stream = ofmt_ctx->streams[pkt->stream_index];
            //out_stream->avg_frame_rate = (AVRational){60, 1};
            //out_stream->time_base = (AVRational){1, 60};

            //out_stream->codec->gop_size = 30;
            //out_stream->codec->max_b_frames = 1;

            //out_stream->codec->framerate = AVRational{30,1};
            //out_stream->time_base = AVRational{1, 30};
            AVRational encoderTimebase = outputVideoCodecContext->time_base;//{1, 30};
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


            writeLock->lock();
            //qDebug() << "Writing Video Packet";
            int ret = av_interleaved_write_frame(ofmt_ctx, outPacket);
            writeLock->unlock();





            //qDebug() << "Wrote video packet ret = " << ret;
            //int ret = av_write_frame(ofmt_ctx, outPacket);

            //int ret = av_write_frame(ofmt_ctx, pkt);
            if (ret < 0)
            {
                qDebug() << "Error muxing packet";
                //break;
            }
            av_packet_unref(pkt);
            av_packet_unref(outPacket);
            //av_packet_free(&outPacket);

            //av_packet_free(&pkt);
            //av_packet_free(&outPacket);
        }

        //static int count = 0;
        //qDebug() << count << "/" << numberOfFrames;
        //if(count > numberOfFrames) break;
        //count++;
    }
    close();
    imageHandler->readImage(nullptr, nullptr, 0);


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
    avformat_close_input(&ifmt_ctx);
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
        s->tcpSocket->myHeader.append(send);
        return 0;
    }
    else
    {
        //Prepends the video header byte needed by socketHandler
        send.prepend(int(1));
        return s->udpSocket->sendDatagram(send);

    }

}

void VideoHandler::toggleGrabFrames(bool a)
{
    mAbortGrabFrames = !a;
}








