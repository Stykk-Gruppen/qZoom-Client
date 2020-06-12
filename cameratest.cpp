#include "cameratest.h"
#include <QtConcurrent/QtConcurrent>
#include <QDebug>




#include "videohandler.h"
//#define STREAM_DURATION   5.0
#define STREAM_FRAME_RATE 10 /* 25 images/s */
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */
#define SCALE_FLAGS SWS_BICUBIC
#define QFRAMES_PER_MOVIE 100












CameraTest::CameraTest(QString cDeviceName, QString aDeviceName, QObject* parent): QObject(parent)
{
    done = false;
    this->cDeviceName = cDeviceName;
    this->aDeviceName = aDeviceName;
    //c->pix_fmt = STREAM_PIX_FMT;

}

void CameraTest::toggleDone() {
    done = !done;
}

int CameraTest::init() {


    //Registrer div ting
    av_register_all();
    avcodec_register_all();
    avdevice_register_all();
    ofmt = NULL;
    ifmt_ctx = NULL;
    ofmt_ctx = NULL;
    int ret, i;

    //Find input video formats
    AVInputFormat* videoInputFormat = av_find_input_format("v4l2");
    if(videoInputFormat == NULL)
    {
        qDebug() << "Not found videoFormat\n";
        return -1;
    }

    //Open VideoInput
    if (avformat_open_input(&ifmt_ctx, cDeviceName.toUtf8().data(), videoInputFormat, NULL) < 0) {
       fprintf(stderr, "Could not open input file '%s'", cDeviceName.toUtf8().data());
       return -1;
    }

    //Get stream information
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        return -1;
    }
    //Print stream information
    av_dump_format(ifmt_ctx, 0, NULL, 0);

    //Allocate outputStreamFormatContext
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
    if (!ofmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        return -1;
    }
    //Set OutputFormat
    ofmt = ofmt_ctx->oformat;
    //Guess format based on filename;
    ofmt = av_guess_format(NULL, filename, NULL);

    //Set Output codecs from guess
    outputVideoCodec = avcodec_find_encoder(ofmt->video_codec);
    //outputAudioCodec = avcodec_find_encoder(ofmt->audio_codec);

    //Allocate CodecContext for outputstreams
    outputVideoCodecContext = avcodec_alloc_context3(outputVideoCodec);

    //Loop gjennom inputstreams
    for (i = 0; (unsigned int)i < ifmt_ctx->nb_streams; i++)
    {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream;

        //Hvis instream er Video
        if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            //Setter av inputcodec og codeccontext, så vi slipper bruke deprecated codec
            inputVideoCodec = avcodec_find_decoder((ifmt_ctx)->streams[i]->codecpar->codec_id);
            inputVideoCodecContext = avcodec_alloc_context3(inputVideoCodec);


            //in_stream->codec->framerate = (AVRational){60, 1};
            //in_stream->time_base = (AVRational){1, 60};
            avcodec_parameters_to_context(inputVideoCodecContext, in_stream->codecpar);
            ret = avcodec_open2(inputVideoCodecContext, inputVideoCodec, NULL);
            //inputVideoCodecContext->time_base = AVRational{1, };


            //in_stream->time_base = (AVRational){1, 60};
            //framerate = (AVRational){60, 1};

            inputVideoCodecContext->time_base = in_stream->time_base;
            inputVideoCodecContext->framerate = in_stream->codec->framerate;

            /*Bare her for å løse errors lenger nede, må finne ordentlig løsning på dette*/
            /*inputVideoCodecContext->pix_fmt = (AVPixelFormat)in_stream->codecpar->format;
            inputVideoCodecContext->width = in_stream->codecpar->width;
            inputVideoCodecContext->height = in_stream->codecpar->height;*/
            /*********************************''*****************************************/


            //Lager ny outputStream
            //Tidligere ble denne laget basert på inputvideocodec, vet ikke hva som er best.
            out_stream = avformat_new_stream(ofmt_ctx, outputVideoCodec);
            //Denne trenger vi senere.
            videoStream = i;
            //Setter div parametere. Kanskje vi må sette fler?
            outputVideoCodecContext->bit_rate = in_stream->codecpar->bit_rate;
            outputVideoCodecContext->width = in_stream->codecpar->width;
            outputVideoCodecContext->height = in_stream->codecpar->height;
            outputVideoCodecContext->pix_fmt = STREAM_PIX_FMT;

            outputVideoCodecContext->time_base = inputVideoCodecContext->time_base;
            outputVideoCodecContext->framerate = inputVideoCodecContext->framerate;
            outputVideoCodecContext->gop_size = 30;
            outputVideoCodecContext->max_b_frames = 1;

            //out_stream->time_base = in_stream->time_base;
            //out_stream->codec->framerate;
            //Kopierer parametere inn i out_stream
            avcodec_parameters_from_context(out_stream->codecpar, outputVideoCodecContext);
            ret = avcodec_open2(outputVideoCodecContext, outputVideoCodec, NULL);

            out_stream->time_base = outputVideoCodecContext->time_base;



            qDebug() << "Fant en videoStream\n";
            qDebug() << in_stream->codec->pix_fmt;
            qDebug() << outputVideoCodecContext->width;
            qDebug() << outputVideoCodecContext->pix_fmt;

            //Sett convert context som brukes ved frame conversion senere.
            img_convert_ctx = sws_getContext(
                        in_stream->codecpar->width,
                        in_stream->codecpar->height,
                        in_stream->codec->pix_fmt,
                        outputVideoCodecContext->width,
                        outputVideoCodecContext->height,
                        outputVideoCodecContext->pix_fmt,
                        SWS_BICUBIC,
                        NULL, NULL, NULL);
            //previous_pts = in_stream->start_time;


        }

        av_dump_format(ifmt_ctx, 0, NULL, 0);

        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            return -1;
        }

        out_stream->codecpar->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    }
    if (!(ofmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);

        if (ret < 0) {
            fprintf(stderr, "Could not open output file '%s'", filename);
            return -1;
        }
    }
    av_dump_format(ofmt_ctx, 0, filename, 1);

    ret = avformat_write_header(ofmt_ctx, NULL);

    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        return -1;
    }


    qDebug() << "InputCodecContext: " << inputVideoCodecContext->time_base.num << "/" << inputVideoCodecContext->time_base.den << "\n";
    qDebug() << "in_stream: " << ifmt_ctx->streams[0]->time_base.num << "/" << ifmt_ctx->streams[0]->time_base.den << "\n";
    qDebug() << "OutCodecContext: " << outputVideoCodecContext->time_base.num << "/" << outputVideoCodecContext->time_base.den << "\n";
    qDebug() << "out_stream: " << ofmt_ctx->streams[0]->time_base.num << "/" << ofmt_ctx->streams[0]->time_base.den << "\n";

    QtConcurrent::run(this, &CameraTest::grabFrames);
    QTimer::singleShot(3000, this, SLOT(toggleDone()));
    return 0;
}

void CameraTest::grabFrames() {

    AVPacket* pkt = av_packet_alloc();
    pkt->size = 0;
    pkt->data = NULL;
    //
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

    while ((ret = av_read_frame(ifmt_ctx, pkt)) >= 0)
    {
        if(inputVideoCodecContext->pix_fmt != STREAM_PIX_FMT)
        {
            //av_grow_packet(pkt, 1842688);
            qDebug() << "kommer inn i videoStreamgreiene\n";
            //videoFrame->pkt_size = pkt->size;
            if(ret < 0)
            {
                qDebug() << "Input Avcodec open failed: " << ret << "\n";
                exit(1);
            }
            qDebug() << "Forbi avodec_open\n";

            ret = avcodec_send_packet(inputVideoCodecContext, pkt);
            if(ret < 0)
            {
                qDebug() << "Send packet error";
                exit(1);
            }
            qDebug() << "Forbi send packet\n";

            ret = avcodec_receive_frame(inputVideoCodecContext, videoFrame);
            if(ret < 0)
            {
                qDebug() << "Recieve frame error";
                exit(1);
            }

            //videoFrame->pts = previous_pts + 1 + skipped_frames;
            //previous_pts = videoFrame->pts;

            //ret = avcodec_decode_video2(ifmt_ctx->streams[videoStream]->codec, videoFrame, &frameFinished, pkt);
            qDebug() << "Etter recieve frame\n";
            if (1)
            {
                int num_bytes = av_image_get_buffer_size(outputVideoCodecContext->pix_fmt,outputVideoCodecContext->width,outputVideoCodecContext->height, 1);
                uint8_t* frame2_buffer = (uint8_t *)av_malloc(num_bytes*sizeof(uint8_t));
                av_image_fill_arrays(scaledFrame->data,scaledFrame->linesize, frame2_buffer, outputVideoCodecContext->pix_fmt, outputVideoCodecContext->width, outputVideoCodecContext->height,1);

                scaledFrame->pts = videoFrame->best_effort_timestamp;
                ret = sws_scale(img_convert_ctx, videoFrame->data,
                                videoFrame->linesize, 0,
                                inputVideoCodecContext->height,
                                scaledFrame->data, scaledFrame->linesize);
                qDebug() << "Etter swsScale\n";

                if(ret < 0)
                {
                    qDebug() << "Error with scale " << ret <<"\n";
                    exit(1);
                }
            }






            AVPacket* outPacket = av_packet_alloc();

            outPacket->data = NULL;
            outPacket->size = 0;
            if(ret < 0) qDebug() << "Output Avcodec open failed: " << ret << "\n";

            ret = avcodec_send_frame(outputVideoCodecContext, scaledFrame);
            if(ret < 0)
            {
                qDebug() << "Error with send frame " << ret <<"\n";

                exit(1);
            }

            ret = avcodec_receive_packet(outputVideoCodecContext, outPacket);
            if(ret < 0)
            {
                qDebug() << "Error with receive packet " << ret <<"\n";
                skipped_frames++;
                continue;
            }
            else
            {
                skipped_frames = 0;

                AVStream *in_stream, *out_stream;
                in_stream  = ifmt_ctx->streams[pkt->stream_index];
                out_stream = ofmt_ctx->streams[pkt->stream_index];
                //out_stream->avg_frame_rate = (AVRational){60, 1};
                //out_stream->time_base = (AVRational){1, 60};

                //out_stream->codec->gop_size = 30;
                //out_stream->codec->max_b_frames = 1;


                AVRational encoderTimebase = inputVideoCodecContext->time_base;//{1, 30};
                AVRational muxerTimebase = outputVideoCodecContext->time_base;

                qDebug() << "InputCodecContext: " << inputVideoCodecContext->time_base.num << "/" << inputVideoCodecContext->time_base.den << "\n";
                qDebug() << "in_stream: " << in_stream->time_base.num << "/" << in_stream->time_base.den << "\n";
                qDebug() << "OutCodecContext: " << outputVideoCodecContext->time_base.num << "/" << outputVideoCodecContext->time_base.den << "\n";
                qDebug() << "out_stream: " << out_stream->time_base.num << "/" << out_stream->time_base.den << "\n";



                /* copy packet */

                pkt->pts = av_rescale_q_rnd(pkt->pts, encoderTimebase, muxerTimebase, (AVRounding) (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                pkt->dts = av_rescale_q_rnd(pkt->dts, encoderTimebase, muxerTimebase, (AVRounding) (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                pkt->duration = av_rescale_q(pkt->duration, encoderTimebase, muxerTimebase);
                pkt->pos = -1;

                qDebug() << "Før Write Frame\n";
                int ret = av_write_frame(ofmt_ctx, outPacket);
                qDebug() << "Etter Write Frame\n";
                //int ret = av_write_frame(ofmt_ctx, pkt);
                if (ret < 0) {
                    qDebug() << "Error muxing packet";
                    //break;
                }
                av_packet_unref(pkt);
                av_packet_unref(outPacket);
                //av_packet_free(&pkt);
                //av_packet_free(&outPacket);
            }
        }
        static int count = 0;
        if(count > 100) break;
        count++;
    }



    av_write_trailer(ofmt_ctx);


    avformat_close_input(&ifmt_ctx);
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF) {
        //return -1;
        //fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
    }

    qDebug() << "Ferdig med grabFrames!!!\n";
}


