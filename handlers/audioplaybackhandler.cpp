#include "audioplaybackhandler.h"

AudioPlaybackHandler::AudioPlaybackHandler(std::mutex* _writeLock, QByteArray* buffer, size_t bufferSize,
                                           QObject *parent) : Playback(_writeLock, buffer, bufferSize, parent)
{
    initAudio(parent);
}

AudioPlaybackHandler::~AudioPlaybackHandler()
{

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



void AudioPlaybackHandler::start()
{
    int error = 0;
    AVFormatContext *inputFormatContext = avformat_alloc_context();
    Q_ASSERT(inputFormatContext);

    uint8_t *avioContextBuffer = reinterpret_cast<uint8_t*>(av_malloc(mBufferSize));
    Q_ASSERT(avioContextBuffer);

    AVIOContext *avioContext = avio_alloc_context(avioContextBuffer, static_cast<int>(mBufferSize), 0, mStruct, &customReadPacket, nullptr, nullptr);
    Q_ASSERT(avioContext);

    inputFormatContext->pb = avioContext;
    error = avformat_open_input(&inputFormatContext, nullptr, nullptr, nullptr);
    if(error < 0)
    {
        char* errbuff = (char *)malloc((1000)*sizeof(char));
        av_strerror(error,errbuff,1000);
        qDebug() << "AVformat open input UDP stream failed" << errbuff;
        exit(1);
    }
    //ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if(error < 0)
    {
        char* errbuff = (char *)malloc((1000)*sizeof(char));
        av_strerror(error,errbuff,1000);
        qDebug() << "AVFormat find udp stream failed" << errbuff;
        exit(1);
    }

    qDebug() << "Dumping audioplayback format";
    av_dump_format(inputFormatContext, 0, NULL, 0);

    AVStream * audio_stream = nullptr;
    for (uint i=0; i < inputFormatContext->nb_streams; ++i) {
        auto	st = inputFormatContext->streams[i];
        //Debug() << st->id << st->index << st->start_time << st->duration << st->codecpar->codec_type;
        if(st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audio_stream = st;
        }
    }

    AVCodecParameters	*audioStreamCodecParameters = audio_stream->codecpar;
    AVCodec* audioDecoderCodec = avcodec_find_decoder(audioStreamCodecParameters->codec_id);
    AVCodecContext *audioDecoderCodecContext = avcodec_alloc_context3(audioDecoderCodec);
    error  = avcodec_parameters_to_context(audioDecoderCodecContext, audio_stream->codecpar);
    Q_ASSERT(error>=0);
    error = avcodec_open2(audioDecoderCodecContext, audioDecoderCodec, nullptr);
    Q_ASSERT(error>=0);

    //Hardcoded sample because the audio_stream codecpar does not contain samplerate
    audioDecoderCodecContext->sample_rate = 48000;
    audioDecoderCodecContext->channels = 2;

    // To initalize libao for playback
    ao_initialize();

    int driver = ao_default_driver_id();

    // The format of the decoded PCM samples
    ao_sample_format sample_format;
    sample_format.bits = 16;
    sample_format.channels = 2;
    sample_format.rate = 48000; //Must match input with 48000 or audio stream delay keeps increasing
    sample_format.byte_format = AO_FMT_NATIVE;
    sample_format.matrix = 0;

    ao_device* device = ao_open_live(driver, &sample_format, NULL);


    // qDebug() << audio_stream->codecpar->sample_rate;
    //qDebug() << audioDecoderCodecContext->sample_rate;
    SwrContext *resample_context = NULL;
    resample_context = swr_alloc_set_opts(NULL,
                                          av_get_default_channel_layout(2),
                                          AV_SAMPLE_FMT_S16,
                                          audioDecoderCodecContext->sample_rate,
                                          av_get_default_channel_layout(audioDecoderCodecContext->channels),
                                          audioDecoderCodecContext->sample_fmt,
                                          audioDecoderCodecContext->sample_rate,
                                          0, NULL);


    //qDebug() << resample_context;
    if (!(resample_context)) {
        fprintf(stderr,"Unable to allocate resampler context\n");
        exit(-1);
        //return AVERROR(ENOMEM);
    }

    // Open the resampler

    if ((error = swr_init(resample_context)) < 0) {
        fprintf(stderr,"Unable to open resampler context: ");
        swr_free(&resample_context);
        exit(-1);
    }

    AVFrame* frame = av_frame_alloc();
    AVPacket packet;
    AVFrame* resampled = 0;

    while (!mStopPlayback) {
        error = av_read_frame(inputFormatContext,&packet);
        if(error < 0)
        {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(error,errbuff,1000);
            qDebug() << "Failed av_read_frame: code " << error << " meaning: " << errbuff;
            int ms = 1000;
            struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
            nanosleep(&ts, NULL);
            continue;
        }
        //qDebug() << "stream: " << packet.stream_index << " mvideostream: " << mVideoStreamIndex;


        error = avcodec_send_packet(audioDecoderCodecContext, &packet);
        if (error == AVERROR_EOF || error == AVERROR(EOF))
        {
            qDebug() << "send packet sleep";
            int ms = 1000;
            struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
            nanosleep(&ts, NULL);
            continue;
        }
        else if(error < 0)
        {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(error,errbuff,1000);
            qDebug() << "Failed udp input avcodec_send_packet: code "<<error<< " meaning: " << errbuff;
            //Endret fra exit til continue for å få mute audio til å fungere.
            //Antakelig fordi den må få lov til å lese ut litt piss før den klarer å finne ny header.
            //exit(1);
            continue;

        }
        error = avcodec_receive_frame(audioDecoderCodecContext, frame);
        if (error == AVERROR(EAGAIN) || error == AVERROR_EOF){
            //skipped_frames++;
            qDebug() << "Skipped a Frame playbackhandler";
            continue;
        }
        else if (error < 0) {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(error,errbuff,1000);
            qDebug() << "Failed avcodec_receive_frame: code "<<error<< " meaning: " << errbuff;
            exit(1);
        }

        if (!resampled)
        {
            resampled = av_frame_alloc();
        }

        resampled->channel_layout = av_get_default_channel_layout(2);
        resampled->sample_rate = audioDecoderCodecContext->sample_rate;
        resampled->format = AV_SAMPLE_FMT_S16;

        if ((error = swr_convert_frame(resample_context, resampled, frame)) < 0)
        {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(error,errbuff,1000);
            qDebug() << "Failed playbackhandler swr_convert_frame: code "<<error<< " meaning: " << errbuff;
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
        av_packet_unref(&packet);

    }

    swr_free(&resample_context);
    avformat_close_input(&inputFormatContext);
    avcodec_free_context(&audioDecoderCodecContext);
    ao_close(device);

}
