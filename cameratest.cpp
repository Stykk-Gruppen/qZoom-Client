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

    //Find Audio Input formats
    AVInputFormat* audioInputFormat = av_find_input_format("alsa");
    if(!(audioInputFormat != NULL))
    {
        qDebug() << "Not found audioFormat\n";
        return -1;
    }

    //Open VideoInput
    if (avformat_open_input(&ifmt_ctx, cDeviceName.toUtf8().data(), videoInputFormat, NULL) < 0) {
       fprintf(stderr, "Could not open input file '%s'", cDeviceName.toUtf8().data());
       return -1;
    }

    //Open AudioInput
    if(avformat_open_input(&ifmt_ctx, aDeviceName.toUtf8().data(), audioInputFormat, NULL) < 0)
    {
        fprintf(stderr, "Could not open audio input file '%s'", aDeviceName.toUtf8().data());
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
    outputAudioCodec = avcodec_find_encoder(ofmt->audio_codec);

    //Allocate CodecContext for outputstreams
    outputVideoCodecContext = avcodec_alloc_context3(outputVideoCodec);
    outputAudioCodecContext = avcodec_alloc_context3(outputAudioCodec);

    //Tror ikke disse er nødvendige
    //video_st.enc = outputVideoCodecContext;
    //video_st.enc->pix_fmt = AV_PIX_FMT_YUV420P;
    //audio_st.enc = outputAudioCodecContext;

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


            /*Bare her for å løse errors lenger nede, må finne ordentlig løsning på dette*/
            inputVideoCodecContext->pix_fmt = (AVPixelFormat)in_stream->codecpar->format;
            inputVideoCodecContext->width = in_stream->codecpar->width;
            inputVideoCodecContext->height = in_stream->codecpar->height;
            /*********************************''*****************************************/


            //Lager ny outputStream
            //Tidligere ble denne laget basert på inputvideocodec, vet ikke hva som er best.
            out_stream = avformat_new_stream(ofmt_ctx, outputVideoCodec);
            //Denne trenger vi senere.
            videoStream = i;
            //Setter div parametere. Kanskje vi må sette fler?
            outputVideoCodecContext->bit_rate = 400000;
            outputVideoCodecContext->width = in_stream->codecpar->width;
            outputVideoCodecContext->height = in_stream->codecpar->height;
            outputVideoCodecContext->pix_fmt = STREAM_PIX_FMT;

            outputVideoCodecContext->time_base = (AVRational){1, 60};
            outputVideoCodecContext->framerate = (AVRational){60, 1};
            outputVideoCodecContext->gop_size = 30;
            outputVideoCodecContext->max_b_frames = 1;

            //Kopierer parametere inn i out_stream
            avcodec_parameters_from_context(out_stream->codecpar, outputVideoCodecContext);

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
        }
        //Hvis inputstream er audio
        else /*if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) */{
            //Setter av inputcodec og codeccontext, så vi slipper bruke deprecated codec
            inputAudioCodec = avcodec_find_decoder((ifmt_ctx)->streams[i]->codecpar->codec_id);
            inputAudioCodecContext = avcodec_alloc_context3(inputAudioCodec);
            //Lager ny outstream basert på outputCodec vi gjettet tidligere
            out_stream = avformat_new_stream(ofmt_ctx, outputAudioCodec);
            avcodec_parameters_to_context(inputAudioCodecContext, in_stream->codecpar);
            //Trenger denne senere.
            audioStream = i;
            //Setter parametere
            outputAudioCodecContext->sample_rate = inputAudioCodecContext->sample_rate;
            outputAudioCodecContext->bit_rate = 96000;
            outputAudioCodecContext->channels = 2;
            outputAudioCodecContext->sample_fmt = outputAudioCodec->sample_fmts[0];
            outputAudioCodecContext->channel_layout = av_get_default_channel_layout(2);
            outputAudioCodecContext->frame_size = in_stream->codecpar->frame_size;
            /* Set the sample rate for the container. */
            out_stream->time_base.den = 48000;
            out_stream->time_base.num = 1;
            //Kopierer parametere inn i out_stream
            avcodec_parameters_from_context(out_stream->codecpar, outputAudioCodecContext);
            //avcodec_parameters_copy(out_stream->codecpar,in_stream->codecpar);
            qDebug() << "Fant en audioStream\n";

            /* create resampler context */
            audioConvertContext = swr_alloc();
            if (!audioConvertContext) {
                fprintf(stderr, "Could not allocate resampler context\n");
                exit(1);
            }

            /* set options */
            av_opt_set_int       (audioConvertContext, "in_channel_count",   outputAudioCodecContext->channels,       0);
            av_opt_set_int       (audioConvertContext, "in_sample_rate",     outputAudioCodecContext->sample_rate,    0);
            av_opt_set_sample_fmt(audioConvertContext, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
            av_opt_set_int       (audioConvertContext, "out_channel_count",  outputAudioCodecContext->channels,       0);
            av_opt_set_int       (audioConvertContext, "out_sample_rate",    outputAudioCodecContext->sample_rate,    0);
            av_opt_set_sample_fmt(audioConvertContext, "out_sample_fmt",     outputAudioCodecContext->sample_fmt,     0);

            /* initialize the resampling context */
            if ((ret = swr_init(audioConvertContext)) < 0) {
                fprintf(stderr, "Failed to initialize the resampling context\n");
                exit(1);
            }
        }

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
    AVFrame* audioFrame;
    audioFrame = av_frame_alloc();
    audioFrame->data[0] = NULL;
    audioFrame->width = inputAudioCodecContext->width;
    audioFrame->height = inputAudioCodecContext->height;
    audioFrame->format = inputAudioCodecContext->pix_fmt;
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


        if(0)
        {
            //av_grow_packet(pkt, 1842688);
            qDebug() << "kommer inn i videoStreamgreiene\n";
            //videoFrame->pkt_size = pkt->size;
            ret = avcodec_open2(inputVideoCodecContext, inputVideoCodec, NULL);
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

            videoFrame->pts = previous_pts + 1 + skipped_frames;
            previous_pts = videoFrame->pts;

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
            ret = avcodec_open2(outputVideoCodecContext, outputVideoCodec, NULL);
            if(ret < 0) qDebug() << "Output Avcodec open failed: " << ret << "\n";

            ret = avcodec_send_frame(outputVideoCodecContext, scaledFrame);
            if(ret < 0)
            {
                qDebug() << "Error with send frame " << ret <<"\n";

                exit(1);
            }
            /*av_packet_unref(pkt);
            pkt = av_packet_alloc();
            pkt->size = 0;
            pkt->data = NULL;

            if(pkt == NULL)
            {
                qDebug() << "pkt = null\n";
                exit(1);
            }
            */
            ret = avcodec_receive_packet(outputVideoCodecContext, outPacket);
            if(ret < 0)
            {
                qDebug() << "Error with receive packet " << ret <<"\n";
                skipped_frames++;
                continue;
            }
            else
            {
                skipped_frames--;

                AVStream *in_stream, *out_stream;
                in_stream  = ifmt_ctx->streams[pkt->stream_index];
                out_stream = ofmt_ctx->streams[pkt->stream_index];
                out_stream->avg_frame_rate = (AVRational){60, 1};
                out_stream->time_base = (AVRational){1, 60};

                out_stream->codec->gop_size = 30;
                out_stream->codec->max_b_frames = 1;


                AVRational encoderTimebase{1, 60};
                AVRational muxerTimebase{1, 1000};



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
        else
        {
            ret = avcodec_open2(inputAudioCodecContext, inputAudioCodec, NULL);
            if(ret < 0)
            {
                qDebug() << "Input Avcodec open failed: " << ret << "\n";
                exit(1);
            }
            qDebug() << "Forbi avodec_open\n";

            ret = avcodec_send_packet(inputAudioCodecContext, pkt);
            if(ret < 0)
            {
                qDebug() << "Send packet error";
                exit(1);
            }
            qDebug() << "Forbi send packet\n";

            ret = avcodec_receive_frame(inputAudioCodecContext, audioFrame);
            if(ret < 0)
            {
                qDebug() << "Recieve frame error";
                exit(1);
            }


        }


        //if(done) break;
        static int count = 0;
        if(count > 300) break;
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

QVariantList CameraTest::getAudioInputDevices()
{
    QList<QVariant> q;
    QList<QAudioDeviceInfo> x = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    for (auto i: x)
    {
        q.append(i.deviceName());
        //todo sjekk om den faktisk er gyldig før den legges til i listen.
    }
    return q;
}

void CameraTest::changeAudioInputDevice(QString deviceName)
{
    qDebug() << deviceName;
    //todo. må vel kanskje kjøre init på nytt?
}


