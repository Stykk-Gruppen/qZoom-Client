#include "videoplaybackhandler.h"
#define 	CODEC_CAP_TRUNCATED   0x0008
#define     CODEC_FLAG_TRUNCATED 0x00010000

VideoPlaybackHandler::VideoPlaybackHandler(std::mutex* _writeLock, QByteArray* buffer,
                                           size_t bufferSize, ImageHandler* _imageHandler,
                                           int index) : Playback(_writeLock, buffer, bufferSize, _imageHandler, index)
{

}

VideoPlaybackHandler::~VideoPlaybackHandler()
{

}


void VideoPlaybackHandler::start()
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

    error = avformat_open_input(&inputFormatContext, NULL, NULL, NULL);
    qDebug() << "HEADER RECEIVED" << Q_FUNC_INFO;

    if(error < 0)
    {
        char* errbuff = (char *)malloc((1000)*sizeof(char));
        av_strerror(error,errbuff,1000);
        qDebug() << "AVformat open input UDP stream failed" << errbuff;
        exit(1);
    }

    //Funker ikke med denne linja, men litt rart det funker uten?
    //error = avformat_find_stream_info(fmt_ctx, nullptr);
    /*if(error < 0)
        {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(error,errbuff,1000);
            qDebug() << "AVFormat find udp stream failed" << errbuff;
            exit(1);
        }*/

    qDebug() << "Dumping videoplayback format";
    av_dump_format(inputFormatContext, 0, NULL, 0);

    AVStream* video_stream = nullptr;
    for (uint i=0; i < inputFormatContext->nb_streams; ++i) {
        auto	st = inputFormatContext->streams[i];
        //Debug() << st->id << st->index << st->start_time << st->duration << st->codecpar->codec_type;
        if (st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream = st;
        }
    }

    int err;
    //Q_ASSERT(video_stream);
    AVCodecContext *videoDecoderCodecContext = video_stream->codec;
    if(video_stream)
    {
        // video_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        AVCodecParameters	*videoStreamCodecParameters = video_stream->codecpar;
        AVCodec* videoDecoderCodec = avcodec_find_decoder(videoStreamCodecParameters->codec_id);

        if (videoDecoderCodec->capabilities & CODEC_CAP_TRUNCATED)
        {
          videoDecoderCodecContext->flags |= CODEC_FLAG_TRUNCATED;
        }
        videoDecoderCodecContext->thread_type  = FF_THREAD_SLICE;
        videoDecoderCodecContext->thread_count = 2;

        videoDecoderCodecContext = avcodec_alloc_context3(videoDecoderCodec);
        videoDecoderCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

        err = avcodec_open2(videoDecoderCodecContext, videoDecoderCodec, nullptr);
        Q_ASSERT(err>=0);
        qDebug() << "codec name: " << videoDecoderCodec->name<< " codec id " << videoDecoderCodec->id;
        qDebug() << "codecpar width" << videoStreamCodecParameters->width <<" h: "<< videoStreamCodecParameters->height << " format: "<< videoStreamCodecParameters->format<< " pix fmt: " << videoDecoderCodecContext->pix_fmt;

        AVFrame	*frameRGB = av_frame_alloc();
        frameRGB->format = AV_PIX_FMT_RGB24;
        frameRGB->width = videoStreamCodecParameters->width;
        frameRGB->height = videoStreamCodecParameters->height;
        err = av_frame_get_buffer(frameRGB, 0);
        Q_ASSERT(err == 0);
    }

    AVFrame* frame = av_frame_alloc();
    AVPacket* packet = av_packet_alloc();
    //AVFrame* resampled = 0;
    mBufferSize = (video_stream->codecpar->bit_rate *2.3) / 1000;
    //mBufferSize = (video_stream->codecpar->width * video_stream->codecpar->height * video_stream->codec->framerate.num * 2) / 1000;
    qDebug() << "Setting buffersize to: " << mBufferSize;

    //while(mStruct->buffer->size() <= mBufferSize);

    while (!mStopPlayback)
    {
        //qDebug() << "About to call av read frame";
        //av_read_frame(fmt_ctx, NULL);

        //AVCodecContext *cctx;
        //inputFormatContext->
        //AVPacket *pkt;
        //AVFrame *frm;
        uint8_t recvbuf[(int)10e5];
        memset(recvbuf, 0, 10e5);
        static int pos = 0;

        AVCodecParserContext * parser = av_parser_init(AV_CODEC_ID_H264);
        parser->flags |= PARSER_FLAG_COMPLETE_FRAMES;
        parser->flags |= PARSER_FLAG_USE_CODEC_TS;




        while(mStruct->buffer->size() <= 0);



        int stringLength = mStruct->buffer->at(0);
        mStruct->writeLock->lock();

        mStruct->buffer->remove(0, 1);

        mStruct->writeLock->unlock();


        qDebug() << "Stringlength: " << stringLength;
        while(mStruct->buffer->size() <= stringLength);


        QByteArray sizeArray = QByteArray(mStruct->buffer->data(), stringLength);
        qDebug() << "sizearray: " << sizeArray;
        QString sizeString = QString(sizeArray);
        qDebug() << "sizestring: " << sizeString;
        int buffer_size = sizeString.toInt();



        while (mStruct->buffer->size() <= buffer_size+stringLength)
        {
            if((*mStruct->stopPlayback))
            {
                //return AVERROR_EOF;
                break;
            }
            //int ms = 5;
            //struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
            //qDebug() << "sleeping";
            //nanosleep(&ts, NULL);
        }


        mStruct->writeLock->lock();
        mStruct->buffer->remove(0, stringLength);

        QByteArray tempBuffer = QByteArray(mStruct->buffer->data(), buffer_size);
        mStruct->buffer->remove(0, buffer_size);
        mStruct->writeLock->unlock();

        memcpy(recvbuf, tempBuffer.constData(), buffer_size);



        //int length = read(, recvbuf, 10e5);
        //int length = mStruct->buffer->size();

        int length = buffer_size;



        if (length >= 0)
        {

            //Creating temporary packet
            AVPacket * tempPacket = new AVPacket;
            av_init_packet(tempPacket);
            av_new_packet(tempPacket, length);
            memcpy(tempPacket->data, recvbuf, length);
            tempPacket->pos = pos;
            pos += length;
            memset(recvbuf,0,length);

            //Parsing temporary packet into pkt
            av_init_packet(packet);






            av_parser_parse2(parser, videoDecoderCodecContext,
                                &(packet->data), &(packet->size),
                                tempPacket->data, tempPacket->size,
                                //AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0
                                tempPacket->pts, tempPacket->dts, tempPacket->pos
                                );


            packet->pts = parser->pts;
            packet->dts = parser->dts;
            packet->pos = parser->pos;

            //Set keyframe flag
            if (parser->key_frame == 1 || (parser->key_frame == -1 && parser->pict_type == AV_PICTURE_TYPE_I))
            {
                packet->flags |= AV_PKT_FLAG_KEY;
            }

            if (parser->key_frame == -1 && parser->pict_type == AV_PICTURE_TYPE_NONE && (packet->flags & AV_PKT_FLAG_KEY))
            {
                packet->flags |= AV_PKT_FLAG_KEY;
            }
            packet->duration = 96000; //Same result as in av_read_frame()

            //Decode:
            error = avcodec_send_packet(videoDecoderCodecContext, packet);
            if (error == AVERROR_EOF || error == AVERROR(EOF))
            {
                qDebug() << "send packet sleep";
                /*int ms = 1000;
                struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
                nanosleep(&ts, NULL);*/
                continue;
            }
            else if(error == AVERROR(EAGAIN))
            {
                qDebug() << "Eagain on send packet!";

            }
            else if(error < 0)
            {
                char* errbuff = (char *)malloc((1000)*sizeof(char));
                av_strerror(error,errbuff,1000);
                qDebug() << "Failed udp input avcodec_send_packet: code "<<error<< " meaning: " << errbuff;
                //exit(1);
                av_packet_unref(packet);

                continue;
            }


            error = avcodec_receive_frame(videoDecoderCodecContext, frame);
            if (error == AVERROR(EAGAIN) || error == AVERROR_EOF){
                //skipped_frames++;
                qDebug() << "Skipped a Frame VideoPlaybackHandler";
                continue;
            }
            else if (error < 0) {
                char* errbuff = (char *)malloc((1000)*sizeof(char));
                av_strerror(error,errbuff,1000);
                qDebug() << "Failed avcodec_receive_frame: code "<<error<< " meaning: " << errbuff;
                exit(1);
            }

            mImageHandler->readImage(videoDecoderCodecContext, frame, mIndex);

            av_frame_unref(frame);
            av_packet_unref(packet);
            //Display frame
            //…




    }


        //qDebug() << inputFormatContext->interrupt_callback.callback;
        /*

        error = av_read_frame(inputFormatContext, &packet);
        //qDebug() << "AVREADFRAME: " << error;
        if(error < 0)
        {
            char* errbuff = (char *)malloc((1000)*sizeof(char));
            av_strerror(error,errbuff,1000);
            qDebug() << "Failed av_read_frame in videoplaybackhandler: code " << error << " meaning: " << errbuff;
            //int ms = 1000;
            //struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
            //nanosleep(&ts, NULL);
            continue;
        }
        */



        //qDebug() << "stream: " << packet.stream_index << " mvideostream: " << mVideoStreamIndex;

        //Decode and send to ImageHandler
        //qDebug() << "packet dts VideoPlaybackHandler: " << packet.dts;
        //qDebug() << "packet pts VideoPlaybackHandler: " << packet.pts;

    }

    avformat_close_input(&inputFormatContext);
    avcodec_free_context(&videoDecoderCodecContext);

}

void VideoPlaybackHandler::decreaseIndex()
{
    mIndex--;
}

/*
int av_read_frame2(AVFormatContext *s, AVPacket *pkt)
 {
      const int genpts = s->flags & AVFMT_FLAG_GENPTS;
      int eof = 0;
      int ret;
      AVStream *st;

      if (!genpts) {
          ret = s->internal->packet_buffer
          ret = s->internal->packet_buffer
                ? read_from_packet_buffer(&s->internal->packet_buffer,
                                          &s->internal->packet_buffer_end, pkt)
                : read_frame_internal(s, pkt);
          if (ret < 0)
              return ret;
          goto return_packet;
      }

      for (;;) {
          AVPacketList *pktl = s->internal->packet_buffer;

          if (pktl) {
              AVPacket *next_pkt = &pktl->pkt;

              if (next_pkt->dts != AV_NOPTS_VALUE) {
                  int wrap_bits = s->streams[next_pkt->stream_index]->pts_wrap_bits;
                  // last dts seen for this stream. if any of packets following
                  // current one had no dts, we will set this to AV_NOPTS_VALUE.
                  int64_t last_dts = next_pkt->dts;
                  while (pktl && next_pkt->pts == AV_NOPTS_VALUE) {
                      if (pktl->pkt.stream_index == next_pkt->stream_index &&
                          (av_compare_mod(next_pkt->dts, pktl->pkt.dts, 2LL << (wrap_bits - 1)) < 0)) {
                          if (av_compare_mod(pktl->pkt.pts, pktl->pkt.dts, 2LL << (wrap_bits - 1))) {
                              // not B-frame
                              next_pkt->pts = pktl->pkt.dts;
                          }
                          if (last_dts != AV_NOPTS_VALUE) {
                              // Once last dts was set to AV_NOPTS_VALUE, we don't change it.
                              last_dts = pktl->pkt.dts;
                          }
                      }
                      pktl = pktl->next;
                  }
                  if (eof && next_pkt->pts == AV_NOPTS_VALUE && last_dts != AV_NOPTS_VALUE) {
                      // Fixing the last reference frame had none pts issue (For MXF etc).
                      // We only do this when
                      // 1. eof.
                      // 2. we are not able to resolve a pts value for current packet.
                      // 3. the packets for this stream at the end of the files had valid dts.
                      next_pkt->pts = last_dts + next_pkt->duration;
                  }
                  pktl = s->internal->packet_buffer;
              }


              st = s->streams[next_pkt->stream_index];
              if (!(next_pkt->pts == AV_NOPTS_VALUE && st->discard < AVDISCARD_ALL &&
                    next_pkt->dts != AV_NOPTS_VALUE && !eof)) {
                  ret = read_from_packet_buffer(&s->internal->packet_buffer,
                                                 &s->internal->packet_buffer_end, pkt);
                  goto return_packet;
              }
          }

          ret = read_frame_internal(s, pkt);
          if (ret < 0) {
              if (pktl && ret != AVERROR(EAGAIN)) {
                  eof = 1;
                  continue;
              } else
                  return ret;
          }

         if (av_dup_packet(add_to_pktbuf(&s->internal->packet_buffer, pkt,
                                          &s->internal->packet_buffer_end)) < 0)
              return AVERROR(ENOMEM);
      }

  return_packet:

      st = s->streams[pkt->stream_index];
      if ((s->iformat->flags & AVFMT_GENERIC_INDEX) && pkt->flags & AV_PKT_FLAG_KEY) {
          ff_reduce_index(s, st->index);
          av_add_index_entry(st, pkt->pos, pkt->dts, 0, 0, AVINDEX_KEYFRAME);
      }

      if (is_relative(pkt->dts))
          pkt->dts -= RELATIVE_TS_BASE;
      if (is_relative(pkt->pts))
          pkt->pts -= RELATIVE_TS_BASE;

      return ret;
 }

*/
