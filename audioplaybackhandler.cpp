#include "audioplaybackhandler.h"

AudioPlaybackHandler::AudioPlaybackHandler(QObject *parent) : QObject(parent)
{
    QtConcurrent::run([this]()
    {
        const char* input_filename = "test.mp3";

        // This call is necessarily done once in your app to initialize
        // libavformat to register all the muxers, demuxers and protocols.
        av_register_all();

        // A media container
        AVFormatContext* container = 0;

        if (avformat_open_input(&container, input_filename, NULL, NULL) < 0) {
            qDebug() << "Could not open file";
        }

        if (avformat_find_stream_info(container, NULL) < 0) {
            qDebug() << "Could not find file info";
        }

        int stream_id = -1;

        // To find the first audio stream. This process may not be necessary
        // if you can gurarantee that the container contains only the desired
        // audio stream
        //int i;
        for (unsigned int i = 0; i < container->nb_streams; i++) {
            if (container->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
                stream_id = i;
                break;
            }
        }

        if (stream_id == -1) {
            qDebug() << "Could not find an audio stream";
        }

        // Extract some metadata
        AVDictionary* metadata = container->metadata;

        const char* artist = av_dict_get(metadata, "artist", NULL, 0)->value;
        const char* title = av_dict_get(metadata, "title", NULL, 0)->value;

        fprintf(stdout, "Playing: %s - %s\n", artist, title);

        // Find the apropriate codec and open it
        AVCodecContext* codec_context = container->streams[stream_id]->codec;

        AVCodec* codec = avcodec_find_decoder(codec_context->codec_id);

        if (avcodec_open2(codec_context, codec, NULL) < 0) {
            qDebug() << "Could not find open the needed codec";
        }

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

        ao_device* device = ao_open_live(driver, &sample_format, NULL);

        AVPacket packet;
        av_init_packet(&packet);
        int buffer_size = 192000;
        //int8_t buffer[192000];
        int ret;
        AVFrame* frame = av_frame_alloc();
        AVFrame* resampled = 0;
        SwrContext *resample_context = NULL;

        resample_context = swr_alloc_set_opts(NULL,
                                               av_get_default_channel_layout(2),
                                               AV_SAMPLE_FMT_S16,
                                               codec_context->sample_rate,
                                               av_get_default_channel_layout(codec_context->channels),
                                               codec_context->sample_fmt,
                                               codec_context->sample_rate,
                                               0, NULL);

            if (!(resample_context)) {
                fprintf(stderr,"Unable to allocate resampler context\n");
                //return AVERROR(ENOMEM);
            }

            // Open the resampler

            if ((ret = swr_init(resample_context)) < 0) {
                fprintf(stderr,"Unable to open resampler context: ");
                swr_free(&resample_context);
            }


        /*
        printf("Initializing resampler...\n");
        if (init_resampler(codec_context, codec_context, &resample_context) < 0)
        {
            avformat_close_input(&container);
            avcodec_free_context(&codec_context);
        }
        */

        while (1) {

            buffer_size = 192000;

            // Read one packet into `packet`
            if (av_read_frame(container, &packet) < 0) {
                break;  // End of stream. Done decoding.
            }

            ret = avcodec_send_packet(codec_context, &packet);
            ret = avcodec_receive_frame(codec_context, frame);


            //int got_frame;
            // Decodes from `packet` into the buffer
            /*
            if (avcodec_decode_audio4(codec_context, frame, &got_frame, &packet) < 1) {
                break;  // Error in decoding
            }
            */

            if (!resampled)
            {
                resampled = av_frame_alloc();
            }

            resampled->channel_layout = av_get_default_channel_layout(2);
            resampled->sample_rate = codec_context->sample_rate;
            resampled->format = AV_SAMPLE_FMT_S16;

            if ((ret = swr_convert_frame(resample_context, resampled, frame)) < 0)
            {
                qDebug() << "Error resampling";
            }
            else
            {
                ao_play(device,(char*)resampled->extended_data[0],resampled->linesize[0]);
                //ao_play(device, (char*)resampled->extended_data[0], av_sample_get_buffer_size(resampled->linesize, resampled->channels, resampled->nb_samples, resampled->format, 0));
            }
            av_frame_unref(resampled);
            av_frame_unref(frame);



            //av_sample_get_buffer_size

            // Send the buffer contents to the audio device
            //ao_play(device, (char*)buffer, buffer_size);
            //ao_play(device, (char*)frame->extended_data[0], av_sample_get_buffer_size(frame->linesize, frame->channels, frame->nb_samples, frame->format, 0));
        }
        avformat_close_input(&container);

        //av_close_input_file(container);

        ao_shutdown();

        fprintf(stdout, "Done playing. Exiting...");
    });

}
