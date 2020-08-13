#include "audioplaybackhandler.h"

AudioPlaybackHandler::AudioPlaybackHandler(std::mutex* _writeLock, QByteArray* buffer,
                                           size_t bufferSize, ImageHandler* _imageHandler,
                                           int index) : Playback(_writeLock, buffer, bufferSize, _imageHandler, index)
{
    mBufferSize = 1024*1;
}

AudioPlaybackHandler::~AudioPlaybackHandler()
{

}
/**
 * Custom readPacket function for av_read_frame and av_open_input.
 * Will read and remove bytes from the buffer found in the struct
 * and copy them to buf.
 * @param buf_size int how many bytes to read from the buffer
 * @param[out] buf uint8_t* bytes to send back to ffmpeg
 * @param opaque void* pointer set by avio_alloc_context
 * @return buf_size int
 */
int AudioPlaybackHandler::customReadPacket(void *opaque, uint8_t *buf, int buf_size)
{
   // static int counter = 0;
    mBufferAndLockStruct *s = reinterpret_cast<mBufferAndLockStruct*>(opaque);
    while (s->buffer->size() <= buf_size)
    {
        if((*s->stopPlayback))
        {
            return AVERROR_EOF;
        }
        //int ms = 5;
        //struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        //qDebug() << "sleeping";
        //nanosleep(&ts, NULL);
    }

    s->writeLock->lock();
    QByteArray tempBuffer = QByteArray(s->buffer->data(), buf_size);
    s->buffer->remove(0, buf_size);
    s->writeLock->unlock();


    memcpy(buf, tempBuffer.constData(), buf_size);

    //Since return value is fixed it will never stop reading, should not be a problem for us?
    //qDebug() << tempBuffer.size();
    //counter++;
    //qDebug() << counter;
    return buf_size;
}
/**
 * Initialize input contexts and starts reading packets from the
 * buffer, applying the filter, resampling the audio and sending it to the speakers
 */
void AudioPlaybackHandler::start()
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

    //AVCodecParameters	*audioStreamCodecParameters = audio_stream->codecpar;

    AVCodec* audioDecoderCodec = avcodec_find_decoder((AVCodecID)86016);
    //AVCodec* audioDecoderCodec = avcodec_find_decoder(audioStreamCodecParameters->codec_id);
    AVCodecContext *audioDecoderCodecContext = avcodec_alloc_context3(audioDecoderCodec);
    // error  = avcodec_parameters_to_context(audioDecoderCodecContext, audio_stream->codecpar);
    Q_ASSERT(error>=0);
    error = avcodec_open2(audioDecoderCodecContext, audioDecoderCodec, nullptr);
    Q_ASSERT(error>=0);

    //Hardcoded sample because the audio_stream codecpar does not contain samplerate
    audioDecoderCodecContext->sample_rate = 48000;
    audioDecoderCodecContext->channels = 2;

    // To initalize libao for playback
    ao_initialize();

    int driver = ao_default_driver_id();
    ao_info *driver_info = ao_driver_info(driver);
        printf("Player audio driver: %s\n", driver_info->name);

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
    //out,out,out...in,in,in
    //qDebug() << "audioDecoderCodecContext->sample_fmt," << audioDecoderCodecContext->sample_fmt;
    resample_context = swr_alloc_set_opts(NULL,
                                          av_get_default_channel_layout(2),
                                          AV_SAMPLE_FMT_S16,
                                          audioDecoderCodecContext->sample_rate,
                                          av_get_default_channel_layout(audioDecoderCodecContext->channels),
                                          //audioDecoderCodecContext->sample_fmt, //unfiltered format
                                          (AVSampleFormat)4, // filtered format
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
    ///////////////FILTER
    ///
    AVFilterGraph *graph;
    AVFilterContext *buffersrc_ctx, *buffersink_ctx;
    AVFrame *filt_frame = av_frame_alloc();

    /* Set up the filtergraph. */
    error = initFilterGraph(&graph, &buffersrc_ctx, audioDecoderCodecContext, &buffersink_ctx);
    if (error < 0) {
        fprintf(stderr, "Unable to init filter graph:");
        exit(-1);
    }

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


        /* push the audio data from decoded frame into the filtergraph */
        error = av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
        if(error<0){
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(error,errbuff,1000);
            qDebug() << "Failed playbackhandler av_buffersrc_add_frame_flags: code "<<error<< " meaning: " << errbuff;
            exit(1);
        }

        // pull filtered audio from the filtergraph
        error = av_buffersink_get_frame(buffersink_ctx, filt_frame);
        if(error<0){
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(error,errbuff,1000);
            qDebug() << "Failed playbackhandler av_buffersink_get_frame: code "<<error<< " meaning: " << errbuff;
            exit(1);
        }

        AVDictionary *meta = filt_frame->metadata;
        if(meta)
        {
            AVDictionaryEntry* silenceData = av_dict_get(meta, "", NULL, AV_DICT_IGNORE_SUFFIX);
            if(silenceData)
            {
                QString silenceStart = "lavfi.silence_start";
                QString silenceEnd = "lavfi.silence_end";
                if(silenceEnd.compare(silenceData->key)==0)
                {
                    mImageHandler->toggleBorder(true,mIndex);
                }
                if(silenceStart.compare(silenceData->key)==0)
                {
                    mImageHandler->toggleBorder(false,mIndex);
                }
            }
        }
        av_dict_free(&meta);

        if (!resampled)
        {
            resampled = av_frame_alloc();
        }

        resampled->channel_layout = av_get_default_channel_layout(2);
        resampled->sample_rate = audioDecoderCodecContext->sample_rate;
        resampled->format = AV_SAMPLE_FMT_S16;

        //qDebug() << "frame fmt" << frame->format;
        //qDebug() << "filt_frame fmt" << filt_frame->format;

        if ((error = swr_convert_frame(resample_context, resampled, filt_frame)) < 0)
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

/**
 * Initialize filter graph for reducing noise
 * and displaying border around the person talking
 * @param ctx AVCodecContext decoder codec context for setting options
 * @param[out] graph AVFilterGraph
 * @param[out] src AVFilterContext
 * @param[out] sink AVFilterContext
 * @return Error code (0 if successful)
 */
int AudioPlaybackHandler::initFilterGraph(AVFilterGraph **graph, AVFilterContext **src,
                                            AVCodecContext *ctx, AVFilterContext **sink)
{
    AVFilterGraph *filter_graph;
    AVFilterContext *abuffer_ctx;
    const AVFilter  *abuffer;
   // AVFilterContext *volume_ctx;
    //const AVFilter  *volume;
    AVFilterContext *silcence_ctx;
    const AVFilter  *silcence;
    AVFilterContext *gate_ctx;
    const AVFilter  *gate;
    AVFilterContext *aformat_ctx;
    const AVFilter  *aformat;
    AVFilterContext *abuffersink_ctx;
    const AVFilter  *abuffersink;

    int err;

    /* Create a new filtergraph, which will contain all the filters. */
    filter_graph = avfilter_graph_alloc();
    if (!filter_graph) {
        fprintf(stderr, "Unable to create filter graph.\n");
        return AVERROR(ENOMEM);
    }

    /* Create the abuffer filter;
     * it will be used for feeding the data into the graph. */
    abuffer = avfilter_get_by_name("abuffer");
    if (!abuffer) {
        fprintf(stderr, "Could not find the abuffer filter.\n");
        return AVERROR_FILTER_NOT_FOUND;
    }

    abuffer_ctx = avfilter_graph_alloc_filter(filter_graph, abuffer, "src");
    if (!abuffer_ctx) {
        fprintf(stderr, "Could not allocate the abuffer instance.\n");
        return AVERROR(ENOMEM);
    }


    av_opt_set_int    (abuffer_ctx, "channels", 2, AV_OPT_SEARCH_CHILDREN);
    av_opt_set    (abuffer_ctx, "channel_layout",  QString::number(av_get_default_channel_layout(2)).toUtf8().data(), AV_OPT_SEARCH_CHILDREN);
    av_opt_set    (abuffer_ctx, "sample_fmt",     av_get_sample_fmt_name(ctx->sample_fmt), AV_OPT_SEARCH_CHILDREN);
    //av_opt_set    (abuffer_ctx, "sample_fmt",     av_get_sample_fmt_name(AV_SAMPLE_FMT_S16), AV_OPT_SEARCH_CHILDREN);
    av_opt_set_q  (abuffer_ctx, "time_base",      (AVRational){ 1, 48000 }, AV_OPT_SEARCH_CHILDREN);
    av_opt_set_int(abuffer_ctx, "sample_rate",    ctx->sample_rate, AV_OPT_SEARCH_CHILDREN);
    /*
    av_opt_set_int    (abuffer_ctx, "channels", ctx->channels, AV_OPT_SEARCH_CHILDREN);
    av_opt_set    (abuffer_ctx, "channel_layout",  QString::number(av_get_default_channel_layout(ctx->channels)).toUtf8().data(), AV_OPT_SEARCH_CHILDREN);
    av_opt_set    (abuffer_ctx, "sample_fmt",     av_get_sample_fmt_name(ctx->sample_fmt), AV_OPT_SEARCH_CHILDREN);
    av_opt_set_q  (abuffer_ctx, "time_base",      ctx->time_base, AV_OPT_SEARCH_CHILDREN);
    av_opt_set_int(abuffer_ctx, "sample_rate",    ctx->sample_rate, AV_OPT_SEARCH_CHILDREN);*/


    /* Now initialize the filter; we pass NULL options, since we have already
     * set all the options above. */
    err = avfilter_init_str(abuffer_ctx, NULL);
    if (err < 0) {
        fprintf(stderr, "Could not initialize the abuffer filter.\n");
        return err;
    }

    gate = avfilter_get_by_name("agate");
    if (!gate) {
        fprintf(stderr, "Could not find the volume filter.\n");
        return AVERROR_FILTER_NOT_FOUND;
    }
    gate_ctx = avfilter_graph_alloc_filter(filter_graph, gate, "agate");
    if (!gate_ctx) {
        fprintf(stderr, "Could not allocate the volume instance.\n");
        return AVERROR(ENOMEM);
    }
    err = avfilter_init_dict(gate_ctx, NULL);
    if (err < 0) {
        fprintf(stderr, "Could not initialize the volume filter.\n");
        return err;
    }

    silcence = avfilter_get_by_name("silencedetect");
    if (!gate) {
        fprintf(stderr, "Could not find the volume filter.\n");
        return AVERROR_FILTER_NOT_FOUND;
    }
    silcence_ctx = avfilter_graph_alloc_filter(filter_graph, silcence, "agate");
    if (!silcence_ctx) {
        fprintf(stderr, "Could not allocate the volume instance.\n");
        return AVERROR(ENOMEM);
    }
    err = avfilter_init_dict(silcence_ctx, NULL);
    if (err < 0) {
        fprintf(stderr, "Could not initialize the volume filter.\n");
        return err;
    }

    /* Create volume filter. */
    // avfilter_get_by_name("silencedetect");

    /* volume = avfilter_get_by_name("volume");
    if (!volume) {
        fprintf(stderr, "Could not find the volume filter.\n");
        return AVERROR_FILTER_NOT_FOUND;
    }

    volume_ctx = avfilter_graph_alloc_filter(filter_graph, volume, "volume");
    if (!volume_ctx) {
        fprintf(stderr, "Could not allocate the volume instance.\n");
        return AVERROR(ENOMEM);
    }


    av_opt_set_double    (volume_ctx, "volume", 0.9, 0);

    err = avfilter_init_dict(volume_ctx, NULL);
    if (err < 0) {
        fprintf(stderr, "Could not initialize the volume filter.\n");
        return err;
    }*/

    /* Create the aformat filter;
     * it ensures that the output is of the format we want. */
    aformat = avfilter_get_by_name("aformat");
    if (!aformat) {
        fprintf(stderr, "Could not find the aformat filter.\n");
        return AVERROR_FILTER_NOT_FOUND;
    }

    aformat_ctx = avfilter_graph_alloc_filter(filter_graph, aformat, "aformat");
    if (!aformat_ctx) {
        fprintf(stderr, "Could not allocate the aformat instance.\n");
        return AVERROR(ENOMEM);
    }


    av_opt_set_int    (aformat_ctx, "channels", 2, AV_OPT_SEARCH_CHILDREN);
    av_opt_set    (aformat_ctx, "channel_layout", QString::number(av_get_default_channel_layout(2)).toUtf8().data(), AV_OPT_SEARCH_CHILDREN);
    av_opt_set    (aformat_ctx, "sample_fmt",     av_get_sample_fmt_name(ctx->sample_fmt), AV_OPT_SEARCH_CHILDREN);
    av_opt_set_int(aformat_ctx, "sample_rate",    ctx->sample_rate, AV_OPT_SEARCH_CHILDREN);

    err = avfilter_init_dict(aformat_ctx, NULL);
    if (err < 0) {
        fprintf(stderr, "Could not initialize the volume filter.\n");
        return err;
    }

    /* Finally create the abuffersink filter;
     * it will be used to get the filtered data out of the graph. */
    abuffersink = avfilter_get_by_name("abuffersink");
    if (!abuffersink) {
        fprintf(stderr, "Could not find the abuffersink filter.\n");
        return AVERROR_FILTER_NOT_FOUND;
    }

    abuffersink_ctx = avfilter_graph_alloc_filter(filter_graph, abuffersink, "sink");
    if (!abuffersink_ctx) {
        fprintf(stderr, "Could not allocate the abuffersink instance.\n");
        return AVERROR(ENOMEM);
    }

    /* This filter takes no options. */
    err = avfilter_init_str(abuffersink_ctx, NULL);
    if (err < 0) {
        fprintf(stderr, "Could not initialize the abuffersink instance.\n");
        return err;
    }

    /* Connect the filters;
     * in this simple case the filters just form a linear chain. */
    err = avfilter_link(abuffer_ctx, 0, aformat_ctx, 0);
    if (err >= 0)
    {
        err = avfilter_link(aformat_ctx, 0, gate_ctx, 0);
    }
    if (err >= 0)
    {
        err = avfilter_link(gate_ctx, 0, silcence_ctx, 0);
    }
    if (err >= 0)
    {
        err = avfilter_link(silcence_ctx, 0, abuffersink_ctx, 0);
    }
    if (err < 0) {
        fprintf(stderr, "Error connecting filters\n");
        return err;
    }

    /* Configure the graph. */
    err = avfilter_graph_config(filter_graph, NULL);
    if (err < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error configuring the filter graph\n");
        return err;
    }

    *graph = filter_graph;
    *src   = abuffer_ctx;
    *sink  = abuffersink_ctx;

    return 0;
}
