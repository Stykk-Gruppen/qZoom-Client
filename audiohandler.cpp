#include "audiohandler.h"
#define OUTPUT_BIT_RATE 96000
/* The number of output channels */
#define OUTPUT_CHANNELS 2
AudioHandler::AudioHandler(QString _cDeviceName, AVFormatContext* _ofmt_ctx, bool _writeToFile, std::mutex* _writeLock, int _numberOfFrames)/*, QObject* parent): QObject(parent)*/
{
    writeToFile = _writeToFile;
    numberOfFrames = _numberOfFrames * 10;
    cDeviceName = _cDeviceName;
    writeLock = _writeLock;
    outputFormatContext = _ofmt_ctx;
    inputFormatContext = NULL;
    inputCodecContext = NULL;
    outputCodecContext = NULL;
    resampleContext = NULL;
    fifo = NULL;
}
int AudioHandler::openInputFile()
{

    av_register_all();
    avcodec_register_all();
    avdevice_register_all();
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
    if ((error = avformat_open_input(&inputFormatContext, cDeviceName.toUtf8().data(), audioInputFormat,
                                     NULL)) < 0) {
        fprintf(stderr, "Could not open audio input context");
        inputFormatContext = NULL;
        return error;
    }

    /* Get information on the input file (number of streams etc.). */
    if ((error = avformat_find_stream_info(inputFormatContext, NULL)) < 0) {
        fprintf(stderr, "Could not find audio stream info ");
        avformat_close_input(&inputFormatContext);
        return error;
    }

    /* Make sure that there is only one stream in the input file. */
    if ((inputFormatContext)->nb_streams != 1) {
        fprintf(stderr, "Expected one audio input stream, but found %d\n",
                (inputFormatContext)->nb_streams);
        avformat_close_input(&inputFormatContext);
        return AVERROR_EXIT;
    }

    /* Find a decoder for the audio stream. */
    if (!(input_codec = avcodec_find_decoder((inputFormatContext)->streams[0]->codecpar->codec_id))) {
        fprintf(stderr, "Could not find audio input codec\n");
        avformat_close_input(&inputFormatContext);
        return AVERROR_EXIT;
    }

    /* Allocate a new decoding context. */
    avctx = avcodec_alloc_context3(input_codec);
    if (!avctx) {
        fprintf(stderr, "Could not allocate a audio decoding context\n");
        avformat_close_input(&inputFormatContext);
        return AVERROR(ENOMEM);
    }

    /* Initialize the stream parameters with demuxer information. */
    error = avcodec_parameters_to_context(avctx, (inputFormatContext)->streams[0]->codecpar);
    if (error < 0) {
        avformat_close_input(&inputFormatContext);
        avcodec_free_context(&avctx);
        return error;
    }

    /* Open the decoder for the audio stream to use it later. */
    if ((error = avcodec_open2(avctx, input_codec, NULL)) < 0) {
        fprintf(stderr, "Could not open audio input codec");
        avcodec_free_context(&avctx);
        avformat_close_input(&inputFormatContext);
        return error;
    }

    /* Save the decoder context for easier access later. */
    inputCodecContext = avctx;

    return 0;
}

/**
 * Open an output file and the required encoder.
 * Also set some basic encoder parameters.
 * Some of these parameters are based on the input file's parameters.
 * @param      filename              File to be opened
 * @param      inputCodecContext   Codec context of input file
 * @param[out] outputFormatContext Format context of output file
 * @param[out] outputCodecContext  Codec context of output file
 * @return Error code (0 if successful)
 */
int AudioHandler::openOutputFile()
{
    AVCodecContext *avctx          = NULL;
    AVIOContext *output_io_context = NULL;
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
    /*if (!(outputFormatContext = avformat_alloc_context())) {
        fprintf(stderr, "Could not allocate output format context\n");
        return AVERROR(ENOMEM);
    }*/

    /* Associate the output file (pointer) with the container format context. */
    //(outputFormatContext)->pb = output_io_context;

    /* Guess the desired container format based on the file extension. */
    /* if (!((outputFormatContext)->oformat = av_guess_format(NULL, filename,
                                                           NULL))) {
        fprintf(stderr, "Could not find output file format\n");
        goto cleanup;
    }*/



    /* Find the encoder to be used by its name. */
    if (!(output_codec = avcodec_find_encoder(AV_CODEC_ID_AAC))) {
        fprintf(stderr, "Could not find an AAC encoder.\n");
        goto cleanup;
    }

    /* Create a new audio stream in the output file container. */
    if (!(stream = avformat_new_stream(outputFormatContext, NULL))) {
        fprintf(stderr, "Could not create new audio stream\n");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }

    avctx = avcodec_alloc_context3(output_codec);
    if (!avctx) {
        fprintf(stderr, "Could not allocate an audio encoding context\n");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }

    /* Set the basic encoder parameters.
     * The input file's sample rate is used to avoid a sample rate conversion. */
    avctx->channels       = OUTPUT_CHANNELS;
    avctx->channel_layout = av_get_default_channel_layout(OUTPUT_CHANNELS);
    avctx->sample_rate    = inputCodecContext->sample_rate;
    avctx->sample_fmt     = output_codec->sample_fmts[0];
    avctx->bit_rate       = OUTPUT_BIT_RATE;

    /* Allow the use of the experimental AAC encoder. */
    avctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    /* Set the sample rate for the container. */
    stream->time_base.den = inputCodecContext->sample_rate;
    stream->time_base.num = 1;

    /* Some container formats (like MP4) require global headers to be present.
     * Mark the encoder so that it behaves accordingly. */
    if ((outputFormatContext)->oformat->flags & AVFMT_GLOBALHEADER)
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
    outputCodecContext = avctx;

    return 0;

cleanup:
    avcodec_free_context(&avctx);
    avio_closep(&(outputFormatContext)->pb);
    avformat_free_context(outputFormatContext);
    outputFormatContext = NULL;
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
 * @param      inputCodecContext  Codec context of the input file
 * @param      outputCodecContext Codec context of the output file
 * @param[out] resampleContext     Resample context for the required conversion
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
    resampleContext = swr_alloc_set_opts(NULL,
                                         av_get_default_channel_layout(outputCodecContext->channels),
                                         outputCodecContext->sample_fmt,
                                         outputCodecContext->sample_rate,
                                         av_get_default_channel_layout(inputCodecContext->channels),
                                         inputCodecContext->sample_fmt,
                                         inputCodecContext->sample_rate,
                                         0, NULL);
    if (!resampleContext) {
        fprintf(stderr, "Could not allocate audio resample context\n");
        return AVERROR(ENOMEM);
    }
    /*
        * Perform a sanity check so that the number of converted samples is
        * not greater than the number of samples to be converted.
        * If the sample rates differ, this case has to be handled differently
        */
    av_assert0(outputCodecContext->sample_rate == inputCodecContext->sample_rate);

    /* Open the resampler with the specified parameters. */
    if ((error = swr_init(resampleContext)) < 0) {
        fprintf(stderr, "Could not open audio resample context\n");
        swr_free(&resampleContext);
        return error;
    }
    return 0;
}

/**
 * Initialize a FIFO buffer for the audio samples to be encoded.
 * @param[out] fifo                 Sample buffer
 * @param      outputCodecContext Codec context of the output file
 * @return Error code (0 if successful)
 */
int AudioHandler::initFifo()
{
    /* Create the FIFO buffer based on the specified output sample format. */
    if (!(fifo = av_audio_fifo_alloc(outputCodecContext->sample_fmt,
                                     outputCodecContext->channels, 1))) {
        fprintf(stderr, "Could not allocate audio FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

/**
 * Write the header of the output file container.
 * @param outputFormatContext Format context of the output file
 * @return Error code (0 if successful)
 */
/*int AudioHandler::writeOutputFileHeader()
{
    int error;
    if ((error = avformat_write_header(outputFormatContext, NULL)) < 0) {
        fprintf(stderr, "Could not write output file header");
        return error;
    }
    return 0;
}*/

/**
 * Decode one audio frame from the input file.
 * @param      frame                Audio frame to be decoded
 * @param      inputFormatContext Format context of the input file
 * @param      inputCodecContext  Codec context of the input file
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
    if ((error = av_read_frame(inputFormatContext, &input_packet)) < 0) {
        /* If we are at the end of the file, flush the decoder below. */
        if (error == AVERROR_EOF)
            *finished = 1;
        else {
            fprintf(stderr, "Could not read audio frame");
            return error;
        }
    }

    /* Send the audio frame stored in the temporary packet to the decoder.
     * The input audio stream decoder is used to do this. */
    if ((error = avcodec_send_packet(inputCodecContext, &input_packet)) < 0) {
        fprintf(stderr, "Could not send audio packet for decoding ");
        return error;
    }

    /* Receive one frame from the decoder. */
    error = avcodec_receive_frame(inputCodecContext, frame);
    /* If the decoder asks for more data to be able to decode a frame,
     * return indicating that no data is present. */
    if (error == AVERROR(EAGAIN)) {
        error = 0;
        goto cleanup;
        /* If the end of the input file is reached, stop decoding. */
    } else if (error == AVERROR_EOF) {
        *finished = 1;
        error = 0;
        goto cleanup;
    } else if (error < 0) {
        fprintf(stderr, "Could not decode audio frame");
        goto cleanup;
        /* Default case: Return decoded data. */
    } else {
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
 * @param      outputCodecContext    Codec context of the output file
 * @param      frame_size              Number of samples to be converted in
 *                                     each round
 * @return Error code (0 if successful)
 */
int AudioHandler::initConvertedSamples(uint8_t ***converted_input_samples,int frame_size)
{
    int error;

    /* Allocate as many pointers as there are audio channels.
     * Each pointer will later point to the audio samples of the corresponding
     * channels (although it may be NULL for interleaved formats).
     */
    if (!(*converted_input_samples = static_cast<uint8_t **>(calloc(outputCodecContext->channels,sizeof(**converted_input_samples)))))
    {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }

    /* Allocate memory for the samples of all channels in one consecutive
     * block for convenience. */
    if ((error = av_samples_alloc(*converted_input_samples, NULL,
                                  outputCodecContext->channels,
                                  frame_size,
                                  outputCodecContext->sample_fmt, 0)) < 0) {
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
 * @param      resampleContext Resample context for the conversion
 * @return Error code (0 if successful)
 */
int AudioHandler::convertSamples(const uint8_t **input_data,
                                 uint8_t **converted_data, const int frame_size)
{
    int error;

    /* Convert the samples using the resampler. */
    if ((error = swr_convert(resampleContext,
                             converted_data, frame_size,
                             input_data    , frame_size)) < 0) {
        fprintf(stderr, "Could not convert input samples");
        return error;
    }

    return 0;
}

/**
 * Add converted input audio samples to the FIFO buffer for later processing.
 * @param fifo                    Buffer to add the samples to
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
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0) {
        fprintf(stderr, "Could not reallocate audio FIFO\n");
        return error;
    }

    /* Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(fifo, (void **)converted_input_samples,
                            frame_size) < frame_size) {
        fprintf(stderr, "Could not write data to audio FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}

/**
 * Read one audio frame from the input file, decode, convert and store
 * it in the FIFO buffer.
 * @param      fifo                 Buffer used for temporary storage
 * @param      inputFormatContext Format context of the input file
 * @param      inputCodecContext  Codec context of the input file
 * @param      outputCodecContext Codec context of the output file
 * @param      resampler_context    Resample context for the conversion
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
    if (!(input_frame = av_frame_alloc())) {
        fprintf(stderr, "Could not allocate audio input frame\n");
        goto cleanup;
    }
    //if (initInputFrame(&input_frame))
    // goto cleanup;
    /* Decode one frame worth of audio samples. */
    if (decodeAudioFrame(input_frame,&data_present, finished)){
        goto cleanup;
    }
    /* If we are at the end of the file and there are no more samples
     * in the decoder which are delayed, we are actually finished.
     * This must not be treated as an error. */
    if (*finished) {
        ret = 0;
        goto cleanup;
    }
    /* If there is decoded data, convert and store it. */
    if (data_present) {
        /* Initialize the temporary storage for the converted input samples. */
        if (initConvertedSamples(&converted_input_samples, input_frame->nb_samples)){
            goto cleanup;
        }

        /* Convert the input samples to the desired output sample format.
         * This requires a temporary storage provided by converted_input_samples. */
        if (convertSamples((const uint8_t**)input_frame->extended_data, converted_input_samples,
                           input_frame->nb_samples)){
            goto cleanup;
        }

        /* Add the converted input samples to the FIFO buffer for later processing. */
        if (addSamplesToFifo(converted_input_samples,input_frame->nb_samples)){
            goto cleanup;
        }

        ret = 0;
    }
    ret = 0;

cleanup:
    if (converted_input_samples) {
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
 * @param      outputCodecContext Codec context of the output file
 * @param      frame_size           Size of the frame
 * @return Error code (0 if successful)
 */
int AudioHandler::initOutputFrame(AVFrame **frame,int frame_size)
{
    int error;

    /* Create a new frame to store the audio samples. */
    if (!(*frame = av_frame_alloc())) {
        fprintf(stderr, "Could not allocate audio output frame\n");
        return AVERROR_EXIT;
    }

    /* Set the frame's parameters, especially its size and format.
     * av_frame_get_buffer needs this to allocate memory for the
     * audio samples of the frame.
     * Default channel layouts based on the number of channels
     * are assumed for simplicity. */
    (*frame)->nb_samples     = frame_size;
    (*frame)->channel_layout = outputCodecContext->channel_layout;
    (*frame)->format         = outputCodecContext->sample_fmt;
    (*frame)->sample_rate    = outputCodecContext->sample_rate;

    /* Allocate the samples of the created frame. This call will make
     * sure that the audio frame can hold as many samples as specified. */
    if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
        fprintf(stderr, "Could not allocate output audio frame samples");
        av_frame_free(frame);
        return error;
    }

    return 0;
}

/* Global timestamp for the audio frames. */
static int64_t pts = 0;
static bool first = true;

/**
 * Encode one frame worth of audio to the output file.
 * @param      frame                 Samples to be encoded
 * @param      outputFormatContext Format context of the output file
 * @param      outputCodecContext  Codec context of the output file
 * @param[out] data_present          Indicates whether data has been
 *                                   encoded
 * @return Error code (0 if successful)
 */
int AudioHandler::encodeAudioFrame(AVFrame *frame,
                                   /*AVFormatContext *outputFormatContext,
                                                                      AVCodecContext *outputCodecContext,*/
                                   int *data_present)
{
    /* Packet used for temporary storage. */
    AVPacket output_packet;
    int error;
    initPacket(&output_packet);

    /* Set a timestamp based on the sample rate for the container. */
    if(first){
            pts = av_gettime()*10;
            first = false;
        }

    if (frame) {
        frame->pts = pts;
        pts += frame->nb_samples;
    }
    //frame->pts = frame->best_effort_timestamp;

    /* Send the audio frame stored in the temporary packet to the encoder.
     * The output audio stream encoder is used to do this. */
    error = avcodec_send_frame(outputCodecContext, frame);
    /* The encoder signals that it has nothing more to encode. */
    if (error == AVERROR_EOF) {
        error = 0;
        goto cleanup;
    } else if (error < 0) {
        fprintf(stderr, "Could not send packet audio for encoding");
        return error;
    }

    /* Receive one encoded frame from the encoder. */
    error = avcodec_receive_packet(outputCodecContext, &output_packet);
    /* If the encoder asks for more data to be able to provide an
     * encoded frame, return indicating that no data is present. */
    if (error == AVERROR(EAGAIN)) {
        error = 0;
        goto cleanup;
        /* If the last frame has been encoded, stop encoding. */
    } else if (error == AVERROR_EOF) {
        error = 0;
        goto cleanup;
    } else if (error < 0) {
        fprintf(stderr, "Could not encode audio frame");
        goto cleanup;
        /* Default case: Return encoded data. */
    } else {
        *data_present = 1;
    }

    /* Write one audio frame from the temporary packet to the output file. */
    writeLock->lock();
    qDebug() << "**********VIDEO*****************";
    qDebug() << "PTS:" << output_packet.pts;
    qDebug() << "DTS:" << output_packet.dts;


    if (*data_present &&
            (error = av_interleaved_write_frame(outputFormatContext, &output_packet)) < 0) {
        fprintf(stderr, "Could not write audio frame");
        goto cleanup;
    }
    writeLock->unlock();

cleanup:
    av_packet_unref(&output_packet);
    return error;
}

/**
 * Load one audio frame from the FIFO buffer, encode and write it to the
 * output file.
 * @param fifo                  Buffer used for temporary storage
 * @param outputFormatContext Format context of the output file
 * @param outputCodecContext  Codec context of the output file
 * @return Error code (0 if successful)
 */
int AudioHandler::loadEncodeAndWrite(/*AVAudioFifo *fifo,
                                                                          AVFormatContext *outputFormatContext,
                                                                          AVCodecContext *outputCodecContext*/)
{
    /* Temporary storage of the output samples of the frame written to the file. */
    AVFrame *output_frame;
    /* Use the maximum number of possible samples per frame.
     * If there is less than the maximum possible frame size in the FIFO
     * buffer use this number. Otherwise, use the maximum possible frame size. */
    const int frame_size = FFMIN(av_audio_fifo_size(fifo),
                                 outputCodecContext->frame_size);
    int data_written;

    /* Initialize temporary storage for one output frame. */
    if (initOutputFrame(&output_frame, frame_size))
        return AVERROR_EXIT;

    /* Read as many samples from the FIFO buffer as required to fill the frame.
     * The samples are stored in the frame temporarily. */
    if (av_audio_fifo_read(fifo, (void **)output_frame->data, frame_size) < frame_size) {
        fprintf(stderr, "Could not read audio data from FIFO\n");
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }

    /* Encode one frame worth of audio samples. */
    if (encodeAudioFrame(output_frame, &data_written)) {
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }
    av_frame_free(&output_frame);
    return 0;
}


void AudioHandler::cleanup()
{
    if (fifo)
        av_audio_fifo_free(fifo);
    swr_free(&resampleContext);
    if (outputCodecContext)
        avcodec_free_context(&outputCodecContext);
    /*if (outputFormatContext) {
        avio_closep(&outputFormatContext->pb);
        avformat_free_context(outputFormatContext);
    }*/
    if (inputCodecContext)
        avcodec_free_context(&inputCodecContext);
    if (inputFormatContext)
        avformat_close_input(&inputFormatContext);
    exit(0);
}
int AudioHandler::init()
{
    int ret = AVERROR_EXIT;

    /* Open the input file for reading. */
    if (openInputFile())
    {
        return ret;
    }
    //qDebug() << "Stream opened";
    /* Open the output file for writing. */
    if (openOutputFile())
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
    while (count < numberOfFrames) {
        count++;
        //qDebug() << count;
        /* Use the encoder's desired frame size for processing. */
        const int output_frame_size = outputCodecContext->frame_size;
        int finished                = 0;

        /* Make sure that there is one frame worth of samples in the FIFO
         * buffer so that the encoder can do its work.
         * Since the decoder's and the encoder's frame size may differ, we
         * need to FIFO buffer to store as many frames worth of input samples
         * that they make up at least one frame worth of output samples. */
        while (av_audio_fifo_size(fifo) < output_frame_size) {
            /* Decode one frame worth of audio samples, convert it to the
             * output sample format and put it into the FIFO buffer. */
            if (readDecodeConvertAndStore(&finished)){
                cleanup();
            }


            /* If we are at the end of the input file, we continue
             * encoding the remaining audio samples to the output file. */
            if (finished)
                break;
        }

        /* If we have enough samples for the encoder, we encode them.
         * At the end of the file, we pass the remaining samples to
         * the encoder. */
        while (av_audio_fifo_size(fifo) >= output_frame_size ||
               (finished && av_audio_fifo_size(fifo) > 0))
            /* Take one frame worth of audio samples from the FIFO buffer,
             * encode it and write it to the output file. */
            if (loadEncodeAndWrite())
            {
                cleanup();
            }

        /* If we are at the end of the input file and have encoded
         * all remaining samples, we can exit this loop and finish. */
        if (finished) {
            int data_written;
            /* Flush the encoder as it may have delayed frames. */
            do {
                data_written = 0;
                if (encodeAudioFrame(NULL, &data_written))
                {
                    cleanup();
                }
            } while (data_written);
            break;
        }
    }

    /**
     * Write the trailer of the output file container.
     * @param outputFormatContext Format context of the output file
     * @return Error code (0 if successful)
     */
    if(writeToFile){
        if ((av_write_trailer(outputFormatContext)) < 0) {
            fprintf(stderr, "Could not write output file trailer");
            cleanup();
        }
    }
    return 0;

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

void AudioHandler::changeAudioInputDevice(QString deviceName)
{
    qDebug() << deviceName;
    //todo. må vel kanskje kjøre init på nytt?
}
