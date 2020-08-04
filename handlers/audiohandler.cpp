#include "audiohandler.h"
AudioHandler::AudioHandler(QString _audioDeviceName, std::mutex* _writeLock,int64_t _time,
                           UdpSocketHandler *_socketHandler, int bufferSize, ImageHandler* _imageHandler)/*, QObject* parent): QObject(parent)*/
{
    imageHandler = _imageHandler;
    mBufferSize = bufferSize;
    mSocketHandler = _socketHandler;
    mTime = _time;
    mAudioDeviceName = _audioDeviceName;
    mWriteLock = _writeLock;
    mInputFormatContext = NULL;
    mInputCodecContext = NULL;
    mOutputCodecContext = NULL;
    mResampleContext = NULL;
    mFifo = NULL;
}

/**
 * Open an input stream and the required decoders.
 * Also set some basic decoders parameters.
 * Some of these parameters are based on the input stream's parameters.
 * @return Error code (0 if successful)
 */
int AudioHandler::openInputStream()
{
    AVCodecContext *avctx;
    AVCodec *input_codec;
    int error;
    AVInputFormat* audioInputFormat = av_find_input_format("alsa");

    if(!(audioInputFormat != NULL))
    {
        qDebug() << "Not found audioFormat\n";
        return -1;
    }


    /* Open the input file to read from it. */
    if ((error = avformat_open_input(&mInputFormatContext, mAudioDeviceName.toUtf8().data(), audioInputFormat,
                                     NULL)) < 0) {
        fprintf(stderr, "Could not open audio input context");
        mInputFormatContext = NULL;
        return error;
    }

    /* Get information on the input file (number of streams etc.). */
    if ((error = avformat_find_stream_info(mInputFormatContext, NULL)) < 0) {
        fprintf(stderr, "Could not find audio stream info ");
        avformat_close_input(&mInputFormatContext);
        return error;
    }

    //Print stream information
    qDebug() << "Dumping audiohandler input";
    av_dump_format(mInputFormatContext, 0, NULL, 0);

    /* Make sure that there is only one stream in the input file. */
    if ((mInputFormatContext)->nb_streams != 1) {
        fprintf(stderr, "Expected one audio input stream, but found %d\n",
                (mInputFormatContext)->nb_streams);
        avformat_close_input(&mInputFormatContext);
        return AVERROR_EXIT;
    }

    /* Find a decoder for the audio stream. */
    if (!(input_codec = avcodec_find_decoder((mInputFormatContext)->streams[0]->codecpar->codec_id))) {
        fprintf(stderr, "Could not find audio input codec\n");
        avformat_close_input(&mInputFormatContext);
        return AVERROR_EXIT;
    }

    /* Allocate a new decoding context. */
    avctx = avcodec_alloc_context3(input_codec);
    if (!avctx) {
        fprintf(stderr, "Could not allocate a audio decoding context\n");
        avformat_close_input(&mInputFormatContext);
        return AVERROR(ENOMEM);
    }

    /* Initialize the stream parameters with demuxer information. */
    error = avcodec_parameters_to_context(avctx, (mInputFormatContext)->streams[0]->codecpar);
    if (error < 0) {
        avformat_close_input(&mInputFormatContext);
        avcodec_free_context(&avctx);
        return error;
    }

    /* Open the decoder for the audio stream to use it later. */
    if ((error = avcodec_open2(avctx, input_codec, NULL)) < 0) {
        fprintf(stderr, "Could not open audio input codec");
        avcodec_free_context(&avctx);
        avformat_close_input(&mInputFormatContext);
        return error;
    }

    /* Save the decoder context for easier access later. */
    mInputCodecContext = avctx;

    return 0;
}

/**
 * Open an output stream and the required encoder.
 * Also set some basic encoder parameters.
 * Some of these parameters are based on the input stream's parameters.
 * @return Error code (0 if successful)
 */
int AudioHandler::openOutputStream()
{
    AVCodecContext *avctx          = NULL;
    AVStream *stream               = NULL;
    AVCodec *output_codec          = NULL;
    int error = 0;

    /* Open the output file to write to it. */
    /*if ((error = avio_open(&output_io_context, filename,
                           AVIO_FLAG_WRITE)) < 0) {
        fprintf(stderr, "Could not open output file");
        return error;
    }*/

    /* Create a new format context for the output container format. */
    error = avformat_alloc_output_context2(&mOutputFormatContext, NULL,"mp3", NULL);
    if (error < 0) {
        fprintf(stderr, "Could not alloc output context");
        exit(1);
    }
    int avio_buffer_size = mBufferSize;
    void* avio_buffer = av_malloc(avio_buffer_size);
    AVIOContext* custom_io = avio_alloc_context (
                (unsigned char*)avio_buffer, avio_buffer_size,
                1,
                (void*) mSocketHandler,
                NULL, &audioCustomSocketWrite, NULL);
    mOutputFormatContext->pb = custom_io;
    av_dict_set(&mOptions, "live", "1", 0);

    /* Associate the output file (pointer) with the container format context. */
    //(outputFormatContext)->pb = output_io_context;

    /* Guess the desired container format based on the file extension. */
     /*if (!((outputFormatContext)->oformat = av_guess_format(NULL, filename,
                                                           NULL))) {
        fprintf(stderr, "Could not find output file format\n");
        goto cleanup;
    }*/



    /* Find the encoder to be used by its name. */
    qDebug() << "*********************audio encoder:" << mOutputFormatContext->oformat->audio_codec;
    if (!(output_codec = avcodec_find_encoder(mOutputFormatContext->oformat->audio_codec)))
    {
        fprintf(stderr, "Could not find an AAC encoder.\n");
        goto cleanup;
    }

    /* Create a new audio stream in the output file container. */
    if (!(stream = avformat_new_stream(mOutputFormatContext, NULL)))
    {
        fprintf(stderr, "Could not create new audio stream\n");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }

    avctx = avcodec_alloc_context3(output_codec);
    if (!avctx)
    {
        fprintf(stderr, "Could not allocate an audio encoding context\n");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }

    /* Set the basic encoder parameters.
     * The input file's sample rate is used to avoid a sample rate conversion. */
    avctx->channels       = 2;
    avctx->channel_layout = av_get_default_channel_layout(2);
    avctx->sample_rate    = mInputCodecContext->sample_rate;
    avctx->sample_fmt     = output_codec->sample_fmts[0];
    avctx->bit_rate       = 48000;

    /* Allow the use of the experimental AAC encoder. */
    avctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    /* Set the sample rate for the container. */
    stream->time_base.den = mInputCodecContext->sample_rate;
    stream->time_base.num = 1;

    /* Some container formats (like MP4) require global headers to be present.
     * Mark the encoder so that it behaves accordingly. */
    if ((mOutputFormatContext)->oformat->flags & AVFMT_GLOBALHEADER)
        avctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    /* Open the encoder for the audio stream to use it later. */
    if ((error = avcodec_open2(avctx, output_codec, NULL)) < 0) {
        fprintf(stderr, "Could not open audio output codec");
        goto cleanup;
    }

    error = avcodec_parameters_from_context(stream->codecpar, avctx);
    if (error < 0) {
        fprintf(stderr, "Could not initialize audio stream parameters\n");
        goto cleanup;
    }

    /* Save the encoder context for easier access later. */
    mOutputCodecContext = avctx;

    return 0;

cleanup:
    avcodec_free_context(&avctx);
    avio_closep(&(mOutputFormatContext)->pb);
    avformat_free_context(mOutputFormatContext);
    mOutputFormatContext = NULL;
    return error < 0 ? error : AVERROR_EXIT;
}

/**
 * Initialize one data packet for reading or writing.
 * @param packet Packet to be initialized
 */
void AudioHandler::initPacket(AVPacket *packet)
{
    av_init_packet(packet);
    /* Set the packet data and size so that it is recognized as being empty. */
    packet->data = NULL;
    packet->size = 0;
}



/**
 * Initialize the audio resampler based on the input and output codec settings.
 * If the input and output sample formats differ, a conversion is required
 * libswresample takes care of this, but requires initialization.
 * @return Error code (0 if successful)
 */
int AudioHandler::initResampler()
{
    int error;

    /*
         * Create a resampler context for the conversion.
         * Set the conversion parameters.
         * Default channel layouts based on the number of channels
         * are assumed for simplicity (they are sometimes not detected
         * properly by the demuxer and/or decoder).
         */
    mResampleContext = swr_alloc_set_opts(NULL,
                                         av_get_default_channel_layout(mOutputCodecContext->channels),
                                         mOutputCodecContext->sample_fmt,
                                         mOutputCodecContext->sample_rate,
                                         av_get_default_channel_layout(mInputCodecContext->channels),
                                         mInputCodecContext->sample_fmt,
                                         mInputCodecContext->sample_rate,
                                         0, NULL);
    if (!mResampleContext)
    {
        fprintf(stderr, "Could not allocate audio resample context\n");
        return AVERROR(ENOMEM);
    }
    /*
        * Perform a sanity check so that the number of converted samples is
        * not greater than the number of samples to be converted.
        * If the sample rates differ, this case has to be handled differently
        */
    av_assert0(mOutputCodecContext->sample_rate == mInputCodecContext->sample_rate);

    /* Open the resampler with the specified parameters. */
    if ((error = swr_init(mResampleContext)) < 0)
    {
        fprintf(stderr, "Could not open audio resample context\n");
        swr_free(&mResampleContext);
        return error;
    }
    return 0;
}

/**
 * Initialize a FIFO buffer for the audio samples to be encoded.
 * @return Error code (0 if successful)
 */
int AudioHandler::initFifo()
{
    /* Create the FIFO buffer based on the specified output sample format. */
    if (!(mFifo = av_audio_fifo_alloc(mOutputCodecContext->sample_fmt,
                                     mOutputCodecContext->channels, 1)))
    {
        fprintf(stderr, "Could not allocate audio FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}


/**
 * Decode one audio frame from the input file.
 * @param      frame                Audio frame to be decoded
 * @param[out] data_present         Indicates whether data has been decoded
 * @param[out] finished             Indicates whether the end of file has
 *                                  been reached and all data has been
 *                                  decoded. If this flag is false, there
 *                                  is more data to be decoded, i.e., this
 *                                  function has to be called again.
 * @return Error code (0 if successful)
 */

int AudioHandler::decodeAudioFrame(AVFrame *frame,int *data_present, int *finished)
{
    /* Packet used for temporary storage. */
    AVPacket input_packet;
    int error;
    initPacket(&input_packet);

    /* Read one audio frame from the input file into a temporary packet. */
    if ((error = av_read_frame(mInputFormatContext, &input_packet)) < 0)
    {
        /* If we are at the end of the file, flush the decoder below. */
        if (error == AVERROR_EOF)
        {
            *finished = 1;
        }
        else
        {
            fprintf(stderr, "Could not read audio frame");
            return error;
        }
    }

    /* Send the audio frame stored in the temporary packet to the decoder.
     * The input audio stream decoder is used to do this. */
    if ((error = avcodec_send_packet(mInputCodecContext, &input_packet)) < 0)
    {
        fprintf(stderr, "Could not send audio packet for decoding ");
        return error;
    }

    /* Receive one frame from the decoder. */
    error = avcodec_receive_frame(mInputCodecContext, frame);
    /* If the decoder asks for more data to be able to decode a frame,
     * return indicating that no data is present. */
    if (error == AVERROR(EAGAIN))
    {
        error = 0;
        goto cleanup;
        /* If the end of the input file is reached, stop decoding. */
    }
    else if (error == AVERROR_EOF)
    {
        *finished = 1;
        error = 0;
        goto cleanup;
    }
    else if (error < 0)
    {
        fprintf(stderr, "Could not decode audio frame");
        goto cleanup;
        /* Default case: Return decoded data. */
    }
    else
    {
        *data_present = 1;
        goto cleanup;
    }

cleanup:
    av_packet_unref(&input_packet);
    return error;
}

/**
 * Initialize a temporary storage for the specified number of audio samples.
 * The conversion requires temporary storage due to the different format.
 * The number of audio samples to be allocated is specified in frame_size.
 * @param[out] converted_input_samples Array of converted samples. The
 *                                     dimensions are reference, channel
 *                                     (for multi-channel audio), sample.
 * @param      frame_size              Number of samples to be converted in
 *                                     each round
 * @return Error code (0 if successful)
 */
int AudioHandler::initConvertedSamples(uint8_t ***converted_input_samples, int frame_size)
{
    int error;

    /* Allocate as many pointers as there are audio channels.
     * Each pointer will later point to the audio samples of the corresponding
     * channels (although it may be NULL for interleaved formats).
     */
    if (!(*converted_input_samples = static_cast<uint8_t **>(calloc(mOutputCodecContext->channels,sizeof(**converted_input_samples)))))
    {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }

    /* Allocate memory for the samples of all channels in one consecutive
     * block for convenience. */
    if ((error = av_samples_alloc(*converted_input_samples, NULL,
                                  mOutputCodecContext->channels,
                                  frame_size,
                                  mOutputCodecContext->sample_fmt, 0)) < 0) {
        fprintf(stderr,
                "Could not allocate converted input samples");
        av_freep(&(*converted_input_samples)[0]);
        free(*converted_input_samples);
        return error;
    }
    return 0;
}

/**
 * Convert the input audio samples into the output sample format.
 * The conversion happens on a per-frame basis, the size of which is
 * specified by frame_size.
 * @param      input_data       Samples to be decoded. The dimensions are
 *                              channel (for multi-channel audio), sample.
 * @param[out] converted_data   Converted samples. The dimensions are channel
 *                              (for multi-channel audio), sample.
 * @param      frame_size       Number of samples to be converted
 * @return Error code (0 if successful)
 */
int AudioHandler::convertSamples(const uint8_t **input_data,
                                 uint8_t **converted_data, const int frame_size)
{
    int error;

    /* Convert the samples using the resampler. */
    if ((error = swr_convert(mResampleContext,
                             converted_data, frame_size,
                             input_data    , frame_size)) < 0) {
        fprintf(stderr, "Could not convert input samples");
        return error;
    }

    return 0;
}

/**
 * Add converted input audio samples to the FIFO buffer for later processing.
 * @param converted_input_samples Samples to be added. The dimensions are channel
 *                                (for multi-channel audio), sample.
 * @param frame_size              Number of samples to be converted
 * @return Error code (0 if successful)
 */
int AudioHandler::addSamplesToFifo(uint8_t **converted_input_samples,
                                   const int frame_size)
{
    int error;

    /* Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples. */
    if ((error = av_audio_fifo_realloc(mFifo, av_audio_fifo_size(mFifo) + frame_size)) < 0)
    {
        fprintf(stderr, "Could not reallocate audio FIFO\n");
        return error;
    }

    /* Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(mFifo, (void **)converted_input_samples,
                            frame_size) < frame_size)
    {
        fprintf(stderr, "Could not write data to audio FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}

/**
 * Read one audio frame from the input stream, decode, convert and store
 * it in the FIFO buffer.
 * @param[out] finished             Indicates whether the end of file has
 *                                  been reached and all data has been
 *                                  decoded. If this flag is false,
 *                                  there is more data to be decoded,
 *                                  i.e., this function has to be called
 *                                  again.
 * @return Error code (0 if successful)
 */
int AudioHandler::readDecodeConvertAndStore(int *finished)
{
    /* Temporary storage of the input samples of the frame read from the file. */
    AVFrame *input_frame = NULL;
    /* Temporary storage for the converted input samples. */
    uint8_t **converted_input_samples = NULL;
    int data_present = 0;
    int ret = AVERROR_EXIT;


    /**
     * Initialize one audio frame for reading from the input file.
     * @param[out] frame Frame to be initialized
     * @return Error code (0 if successful)
     */
    /* Initialize temporary storage for one input frame. */
    if (!(input_frame = av_frame_alloc()))
    {
        fprintf(stderr, "Could not allocate audio input frame\n");
        goto cleanup;
    }
    //if (initInputFrame(&input_frame))
    // goto cleanup;
    /* Decode one frame worth of audio samples. */
    if (decodeAudioFrame(input_frame,&data_present, finished))
    {
        goto cleanup;
    }
    /* If we are at the end of the file and there are no more samples
     * in the decoder which are delayed, we are actually finished.
     * This must not be treated as an error. */
    if (*finished)
    {
        ret = 0;
        goto cleanup;
    }
    /* If there is decoded data, convert and store it. */
    if (data_present)
    {
        /* Initialize the temporary storage for the converted input samples. */
        if (initConvertedSamples(&converted_input_samples, input_frame->nb_samples))
        {
            goto cleanup;
        }

        /* Convert the input samples to the desired output sample format.
         * This requires a temporary storage provided by converted_input_samples. */
        if (convertSamples((const uint8_t**)input_frame->extended_data, converted_input_samples,
                           input_frame->nb_samples))
        {
            goto cleanup;
        }

        /* Add the converted input samples to the FIFO buffer for later processing. */
        if (addSamplesToFifo(converted_input_samples,input_frame->nb_samples))
        {
            goto cleanup;
        }

        ret = 0;
    }
    ret = 0;

cleanup:
    if (converted_input_samples)
    {
        av_freep(&converted_input_samples[0]);
        free(converted_input_samples);
    }
    av_frame_free(&input_frame);

    return ret;
}

/**
 * Initialize one input frame for writing to the output file.
 * The frame will be exactly frame_size samples large.
 * @param[out] frame                Frame to be initialized
 * @param      frame_size           Size of the frame
 * @return Error code (0 if successful)
 */
int AudioHandler::initOutputFrame(AVFrame **frame, int frame_size)
{
    int error;

    /* Create a new frame to store the audio samples. */
    if (!(*frame = av_frame_alloc()))
    {
        fprintf(stderr, "Could not allocate audio output frame\n");
        return AVERROR_EXIT;
    }

    /* Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity. */
    (*frame)->nb_samples     = frame_size;
    (*frame)->channel_layout = mOutputCodecContext->channel_layout;
    (*frame)->format         = mOutputCodecContext->sample_fmt;
    (*frame)->sample_rate    = mOutputCodecContext->sample_rate;

    /* Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified. */
    if ((error = av_frame_get_buffer(*frame, 0)) < 0)
    {
        fprintf(stderr, "Could not allocate output audio frame samples");
        av_frame_free(frame);
        return error;
    }

    return 0;
}

int AudioHandler::init_filter_graph(AVFilterGraph **graph,
                                     AVFilterContext **src,
                                     AVFilterContext **sink)
{
    AVFilterGraph *filter_graph;
    AVFilterContext *abuffer_ctx;
    const AVFilter  *abuffer;
    AVFilterContext *volume_ctx;
    const AVFilter  *volume;
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
    av_opt_set    (abuffer_ctx, "sample_fmt",     av_get_sample_fmt_name(mOutputCodecContext->sample_fmt), AV_OPT_SEARCH_CHILDREN);
    //av_opt_set    (abuffer_ctx, "sample_fmt",     av_get_sample_fmt_name(AV_SAMPLE_FMT_S16), AV_OPT_SEARCH_CHILDREN);
    av_opt_set_q  (abuffer_ctx, "time_base",      (AVRational){ 1, 48000 }, AV_OPT_SEARCH_CHILDREN);
    av_opt_set_int(abuffer_ctx, "sample_rate",    mOutputCodecContext->sample_rate, AV_OPT_SEARCH_CHILDREN);
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
    av_opt_set    (aformat_ctx, "sample_fmt",     av_get_sample_fmt_name(mOutputCodecContext->sample_fmt), AV_OPT_SEARCH_CHILDREN);
    av_opt_set_int(aformat_ctx, "sample_rate",    mOutputCodecContext->sample_rate, AV_OPT_SEARCH_CHILDREN);

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
/* Global timestamp for the audio frames. */
static int64_t pts = 0;
static bool first = true;

/**
 * Encode one frame worth of audio to the output file.
 * @param      frame                 Samples to be encoded
 * @param[out] data_present          Indicates whether data has been
 *                                   encoded
 * @return Error code (0 if successful)
 */
int AudioHandler::encodeAudioFrame(AVFrame *frame,int *data_present)
{
    int error;

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
                imageHandler->toggleBorder(false, std::numeric_limits<uint8_t>::max());
            }
            if(silenceStart.compare(silenceData->key)==0)
            {
                imageHandler->toggleBorder(true, std::numeric_limits<uint8_t>::max());
            }
        }
    }



    /* Packet used for temporary storage. */
    AVPacket output_packet;

    initPacket(&output_packet);

    /* Set a timestamp based on the sample rate for the container. */
    if(first)
    {
        pts = mTime * 10;
        first = false;
    }

    if (frame)
    {
        frame->pts = pts;
        pts += frame->nb_samples;
    }
    //frame->pts = frame->best_effort_timestamp;

    /* Send the audio frame stored in the temporary packet to the encoder.
     * The output audio stream encoder is used to do this. */
    error = avcodec_send_frame(mOutputCodecContext, frame);
    /* The encoder signals that it has nothing more to encode. */
    if (error == AVERROR_EOF)
    {
        error = 0;
        goto cleanup;
    }
    else if (error < 0)
    {
        fprintf(stderr, "Could not send packet audio for encoding");
        return error;
    }

    /* Receive one encoded frame from the encoder. */
    error = avcodec_receive_packet(mOutputCodecContext, &output_packet);
    /* If the encoder asks for more data to be able to provide an
     * encoded frame, return indicating that no data is present. */
    if (error == AVERROR(EAGAIN))
    {
        error = 0;
        goto cleanup;
        /* If the last frame has been encoded, stop encoding. */
    }
    else if (error == AVERROR_EOF)
    {
        error = 0;
        goto cleanup;
    }
    else if (error < 0)
    {
        fprintf(stderr, "Could not encode audio frame");
        goto cleanup;
        /* Default case: Return encoded data. */
    }
    else
    {
        *data_present = 1;
    }

    /* Write one audio frame from the temporary packet to the output file. */
    mWriteLock->lock();
    //    qDebug() << "**********AUDIO*****************";
    //    qDebug() << "PTS:" << output_packet.pts;
    //    qDebug() << "DTS:" << output_packet.dts;
    //    qDebug() << output_packet.stream_index;
    //qDebug() << "writing interleaved";
    if (*data_present &&
            (error = av_interleaved_write_frame(mOutputFormatContext, &output_packet)) < 0)
    {
        fprintf(stderr, "Could not write audio frame");
        goto cleanup;
    }
    //qDebug() << error;
    mWriteLock->unlock();

cleanup:
    av_packet_unref(&output_packet);
    return error;
}

/**
 * Load one audio frame from the FIFO buffer, encode and write it to the
 * output file.
 * @return Error code (0 if successful)
 */
int AudioHandler::loadEncodeAndWrite()
{
    /* Temporary storage of the output samples of the frame written to the file. */
    AVFrame *output_frame;
    /* Use the maximum number of possible samples per frame.
     * If there is less than the maximum possible frame size in the FIFO
     * buffer use this number. Otherwise, use the maximum possible frame size. */
    const int frame_size = FFMIN(av_audio_fifo_size(mFifo),
                                 mOutputCodecContext->frame_size);
    int data_written;

    /* Initialize temporary storage for one output frame. */
    if (initOutputFrame(&output_frame, frame_size))
    {
        return AVERROR_EXIT;
    }

    /* Read as many samples from the FIFO buffer as required to fill the frame.
     * The samples are stored in the frame temporarily. */
    if (av_audio_fifo_read(mFifo, (void **)output_frame->data, frame_size) < frame_size)
    {
        fprintf(stderr, "Could not read audio data from FIFO\n");
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }

    /* Encode one frame worth of audio samples. */
    if (encodeAudioFrame(output_frame, &data_written))
    {
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }
    av_frame_free(&output_frame);
    return 0;
}


void AudioHandler::cleanup()
{
    if (mFifo)
    {
        av_audio_fifo_free(mFifo);
    }
    swr_free(&mResampleContext);
    if (mOutputCodecContext)
    {
        avcodec_free_context(&mOutputCodecContext);
    }
    /*if (outputFormatContext) {
        avio_closep(&outputFormatContext->pb);
        avformat_free_context(outputFormatContext);
    }*/
    if (mInputCodecContext)
    {
        avcodec_free_context(&mInputCodecContext);
    }
    if (mInputFormatContext)
    {
        avformat_close_input(&mInputFormatContext);
    }
    exit(0);
}
int AudioHandler::init()
{
    int ret = AVERROR_EXIT;
    qDebug() << "audio init";
    /* Open the input file for reading. */
    if (openInputStream())
    {
        return ret;
    }
    //qDebug() << "Stream opened";
    /* Open the output file for writing. */
    if (openOutputStream())
    {
        return ret;
    }
    //qDebug() << "Output File opened";
    /* Initialize the resampler to be able to convert audio sample formats. */
    if (initResampler())
    {
        return ret;
    }
    //qDebug() << "Sampler init";
    /* Initialize the FIFO buffer to store audio samples to be encoded. */
    if (initFifo())
    {
        return ret;
    }

    qDebug() << "dumping audiohandler output";
    av_dump_format(mOutputFormatContext, 0, NULL, 1);

    ret = avformat_write_header(mOutputFormatContext, &mOptions);
    if(ret<0)
    {
        fprintf(stderr, "Could not open write header");
        exit(-1);
    }

    /* Set up the filtergraph. */
    ret = init_filter_graph(&graph, &buffersrc_ctx, &buffersink_ctx);
    if (ret < 0) {
        fprintf(stderr, "Unable to init filter graph:");
        exit(-1);
    }
    //qDebug() << "Fifo init";
    /* Write the header of the output file container. */
    /*if (writeOutputFileHeader())
    {
        cleanup();
    }
    qDebug() << "Header written";*/
    /* Loop as long as we have input samples to read or output samples
     * to write; abort as soon as we have neither. */
    // int looping = 0;
    return 0;
}

int AudioHandler::grabFrames()
{
    int count = 0;
    mActive = true;
    while (!mAbortGrabFrames)
    {
        count++;
        //qDebug() << count;
        /* Use the encoder's desired frame size for processing. */
        const int output_frame_size = mOutputCodecContext->frame_size;
        int finished = 0;

        /* Make sure that there is one frame worth of samples in the FIFO
         * buffer so that the encoder can do its work.
         * Since the decoder's and the encoder's frame size may differ, we
         * need to FIFO buffer to store as many frames worth of input samples
         * that they make up at least one frame worth of output samples. */
        if(mAbortGrabFrames) break;
        while (av_audio_fifo_size(mFifo) < output_frame_size)
        {
            if(mAbortGrabFrames) break;

            //qDebug() << "fifo < outputframe zie";
            /* Decode one frame worth of audio samples, convert it to the
             * output sample format and put it into the FIFO buffer. */
            if (readDecodeConvertAndStore(&finished))
            {
                cleanup();
            }


            /* If we are at the end of the input file, we continue
             * encoding the remaining audio samples to the output file. */
            if (finished)
            {
                break;
            }
        }

        /* If we have enough samples for the encoder, we encode them.
         * At the end of the file, we pass the remaining samples to
         * the encoder. */
        if(mAbortGrabFrames) break;

        while (av_audio_fifo_size(mFifo) >= output_frame_size ||
               (finished && av_audio_fifo_size(mFifo) > 0))
        {
            if(mAbortGrabFrames) break;

            /* Take one frame worth of audio samples from the FIFO buffer,
             * encode it and write it to the output file. */
            if (loadEncodeAndWrite())
            {
                cleanup();
            }
        }

        /* If we are at the end of the input file and have encoded
         * all remaining samples, we can exit this loop and finish. */
        if (finished)
        {
            int data_written;
            /* Flush the encoder as it may have delayed frames. */
            do
            {
                data_written = 0;
                if (encodeAudioFrame(NULL, &data_written))
                {
                    cleanup();
                }
            } while (data_written);
            break;
        }
    }
    avformat_close_input(&mInputFormatContext);
    mActive = false;

    //cleanup();
    return 0;
    /**
     * Write the trailer of the output file container.
     * @param outputFormatContext Format context of the output file
     * @return Error code (0 if successful)
     */
    /*
    if ((av_write_trailer(outputFormatContext)) < 0)
    {
        fprintf(stderr, "Could not write output file trailer");
        cleanup();
    }
    return 0;
    */

}

QVariantList AudioHandler::getAudioInputDevices()
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

bool AudioHandler::isActive()
{
    return mActive;
}

void AudioHandler::changeAudioInputDevice(QString deviceName)
{
    mAudioDeviceName = deviceName;
    qDebug() << "Changed audio device";
}
int AudioHandler::audioCustomSocketWrite(void* opaque, uint8_t *buffer, int buffer_size)
{
    UdpSocketHandler* socketHandler = reinterpret_cast<UdpSocketHandler*>(opaque);
    char *cptr = reinterpret_cast<char*>(const_cast<uint8_t*>(buffer));
    QByteArray send;
    send = QByteArray(reinterpret_cast<char*>(cptr), buffer_size);
    //qDebug() << "written to socket";
    send.prepend(int(0));
    return socketHandler->sendDatagram(send);
}

void AudioHandler::toggleGrabFrames(bool a)
{
    mAbortGrabFrames = !a;
}


