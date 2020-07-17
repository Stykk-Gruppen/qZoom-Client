#include "audioplaybackhandler.h"

AudioPlaybackHandler::AudioPlaybackHandler(std::mutex* writeLock,ImageHandler* _imageHandler, SocketHandler* _socketHandler, QObject *parent)
{
    mSocketHandler = _socketHandler;
    mImageHandler = _imageHandler;
    initAudio(parent);
    mStruct = new SocketAndIDStruct();
    mStruct->socketHandler = _socketHandler;
    mStruct->writeLock = writeLock;

    connect(mSocketHandler, &SocketHandler::startAudioPlayback, this, &AudioPlaybackHandler::start);
}

void AudioPlaybackHandler::initAudio(QObject *parent)
{
    mAudioFormat.setSampleRate(44100);
    mAudioFormat.setChannelCount(2);
    mAudioFormat.setCodec("audio/pcm");
    mAudioFormat.setSampleType(QAudioFormat::SignedInt);
    mAudioFormat.setSampleSize(16);
    mAudioFormat.setByteOrder(QAudioFormat::LittleEndian);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(mAudioFormat))
    {
        qDebug() << "Raw audio format not supported by backend, cannot play audio.";
    }

    mpAudio = new QAudioOutput(mAudioFormat, parent);
    mpOut = mpAudio->start();
}

/*
void PlaybackHandler::changeSpeaker()
{
    //todo
}
*/


int AudioPlaybackHandler::read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    qDebug() << buf_size;
    SocketAndIDStruct *s = reinterpret_cast<SocketAndIDStruct*>(opaque);

    //buf_size = FFMIN(buf_size, s->socketHandler->mBuffer.size());


    while (s->socketHandler->mAudioBuffer.size() <= buf_size)
    {
        int ms = 5;
        struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        //qDebug() << "sleeping";
        //nanosleep(&ts, NULL);
    }

    s->writeLock->lock();

    QByteArray tempBuffer = QByteArray(s->socketHandler->mAudioBuffer.data(), buf_size);
    s->socketHandler->mAudioBuffer.remove(0,buf_size);

    s->writeLock->unlock();
    //qDebug() << " buffer after removal: " << s->socketHandler->mBuffer.size();


    memcpy(buf, tempBuffer.constData(), buf_size);

    //mSenderId = something;
    //qDebug() << "Reading packet";
    return buf_size;
}

int AudioPlaybackHandler::start()
{
    QtConcurrent::run([this]()
    {
        //int ms = 10000;
        // struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        //nanosleep(&ts, NULL);

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
        //ret = avformat_find_stream_info(fmt_ctx, nullptr);
        if(ret < 0)
        {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(ret,errbuff,1000);
            qDebug() << "AVFormat find udp stream failed" << errbuff;
            exit(1);
        }

        qDebug() << "Dumping audioplayback format";
        av_dump_format(fmt_ctx, 0, NULL, 0);

        AVStream * audio_stream = nullptr;
        for (uint i=0; i < fmt_ctx->nb_streams; ++i) {
            auto	st = fmt_ctx->streams[i];
            //Debug() << st->id << st->index << st->start_time << st->duration << st->codecpar->codec_type;
            if(st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                audio_stream = st;
                mAudioStreamIndex = i;
            }
        }


        int err;
        AVCodecContext *audioDecoderCodecContext;
        ao_device* device;


        AVCodecParameters	*audioStreamCodecParameters = audio_stream->codecpar;
        AVCodec* audioDecoderCodec = avcodec_find_decoder(audioStreamCodecParameters->codec_id);
        audioDecoderCodecContext = avcodec_alloc_context3(audioDecoderCodec);
        err  = avcodec_parameters_to_context(audioDecoderCodecContext, audio_stream->codecpar);
        Q_ASSERT(err>=0);
        err = avcodec_open2(audioDecoderCodecContext, audioDecoderCodec, nullptr);
        Q_ASSERT(err>=0);

        audioDecoderCodecContext->sample_rate = 48000;
        audioDecoderCodecContext->channels = 2;
        // To initalize libao for playback
        ao_initialize();

        int driver = ao_default_driver_id();

        // The format of the decoded PCM samples
        ao_sample_format sample_format;
        sample_format.bits = 16;
        sample_format.channels = 2;
        sample_format.rate = 44100;
        sample_format.byte_format = AO_FMT_NATIVE;
        sample_format.matrix = 0;

        device = ao_open_live(driver, &sample_format, NULL);


        qDebug() << audio_stream->codecpar->sample_rate;
        qDebug() << audioDecoderCodecContext->sample_rate;
        SwrContext *resample_context = NULL;
        resample_context = swr_alloc_set_opts(NULL,
                                              av_get_default_channel_layout(2),
                                              AV_SAMPLE_FMT_S16,
                                              audioDecoderCodecContext->sample_rate,
                                              av_get_default_channel_layout(audioDecoderCodecContext->channels),
                                              audioDecoderCodecContext->sample_fmt,
                                              audioDecoderCodecContext->sample_rate,
                                              0, NULL);


        qDebug() << resample_context;
        if (!(resample_context)) {
            fprintf(stderr,"Unable to allocate resampler context\n");
            exit(-1);
            //return AVERROR(ENOMEM);
        }

        // Open the resampler

        if ((ret = swr_init(resample_context)) < 0) {
            fprintf(stderr,"Unable to open resampler context: ");
            swr_free(&resample_context);
            exit(-1);
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

            if(packet.stream_index == mAudioStreamIndex)
            {
                ret = avcodec_send_packet(audioDecoderCodecContext, &packet);
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
                ret = avcodec_receive_frame(audioDecoderCodecContext, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
                    //skipped_frames++;
                    qDebug() << "Skipped a Frame playbackhandler";
                    continue;
                }
                else if (ret < 0) {
                    char* errbuff = (char *)malloc((1000)*sizeof(char));
                    av_strerror(ret,errbuff,1000);
                    qDebug() << "Failed avcodec_receive_frame: code "<<ret<< " meaning: " << errbuff;
                    exit(1);
                }

                if (!resampled)
                {
                    resampled = av_frame_alloc();
                }

                resampled->channel_layout = av_get_default_channel_layout(2);
                resampled->sample_rate = audioDecoderCodecContext->sample_rate;
                resampled->format = AV_SAMPLE_FMT_S16;

                if ((ret = swr_convert_frame(resample_context, resampled, frame)) < 0)
                {
                    char* errbuff = (char *)malloc((1000)*sizeof(char));
                    av_strerror(ret,errbuff,1000);
                    qDebug() << "Failed playbackhandler swr_convert_frame: code "<<ret<< " meaning: " << errbuff;
                    exit(1);
                }
                else
                {
                    //ao_play(device,(char*)resampled->data[0], resampled->linesize[0]);
                    ao_play(device, (char*)resampled->extended_data[0],
                            av_samples_get_buffer_size(resampled->linesize,
                                                       resampled->channels,
                                                       resampled->nb_samples,
                                                       (AVSampleFormat)resampled->format,
                                                       0));
                }
                av_frame_unref(resampled);
                av_frame_unref(frame);
            }
            else
            {
                qDebug() << "Vi er fucked";
            }


            av_frame_unref(frame);
            av_packet_unref(&packet);

        }
    });
}

