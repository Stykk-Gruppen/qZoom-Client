#include "streamhandler.h"

std::ofstream outfile("videoStream.ismv", std::ostream::binary);



StreamHandler::StreamHandler(ImageHandler* _imageHandler, SocketHandler* _socketHandler)
{
    avcodec_register_all();
    av_register_all();
    int ret;
    ofmt_ctx = NULL;

    mAudioEnabled = true;
    mVideoEnabled = false;
    writeToFile = false;
    numberOfFrames = 200;
    mSocketHandler = _socketHandler;
    if(writeToFile)
    {
        ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
        if (ret < 0) {
            fprintf(stderr, "Could not alloc output context with file '%s'", filename);
            exit(1);
        }

    }
    else
    {
        // ofmt_ctx = avformat_alloc_context();
        ret = avformat_alloc_output_context2(&ofmt_ctx, NULL,"mp3", NULL);
        if (ret < 0) {
            fprintf(stderr, "Could not alloc output context with file '%s'", filename);
            exit(1);
        }
    }
    // ofmt_ctx->oformat = av_guess_format(NULL, NULL, NULL);

    AVDictionary *options = NULL;

    if (writeToFile)
    {
        if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        {
            ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);

            if (ret < 0) {
                fprintf(stderr, "Could not open output file '%s'", filename);
                exit(1);
            }
        }
    }
    else
    {
        //ofmt_ctx->oformat->flags |= AVFMT_NOFILE;

        int avio_buffer_size = 4 * 1024;
        void* avio_buffer = av_malloc(avio_buffer_size);
        AVIOContext* custom_io = avio_alloc_context (
                    (unsigned char*)avio_buffer, avio_buffer_size,
                    1,
                    (void*) mSocketHandler,
                    NULL, &custom_io_write, NULL);
        ofmt_ctx->pb = custom_io;
        av_dict_set(&options, "live", "1", 0);
    }

    int64_t time = av_gettime();
    if(mAudioEnabled && !mVideoEnabled)
    {
        mAudioOutputStreamIndex = 0;
    }
    videoHandler = new VideoHandler("/dev/video0", ofmt_ctx, writeToFile, &writeLock, time,numberOfFrames, _imageHandler);
    audioHandler = new AudioHandler("default", ofmt_ctx, writeToFile, &writeLock,time, numberOfFrames, mAudioOutputStreamIndex);
    if(mVideoEnabled)
    {
        ret = videoHandler->init();
        if(ret<0)
        {
            fprintf(stderr, "Could not init videohandler");
            exit(1);
        }
    }
    else
    {
        qDebug() << "Video has been disabled in streamhandler";
    }
    if(mAudioEnabled)
    {
        ret = audioHandler->init();
        if(ret<0){
            fprintf(stderr, "Could not init audiohandler");
            exit(1);
        }
    }
    else
    {
        qDebug() << "Audio has been disabled in streamhandler";
    }
    if(!mAudioEnabled && !mVideoEnabled)
    {
        qDebug() << "Both audio and video has been disabled in streamhandler";
        exit(1);
    }
    if(writeToFile)
    {
        av_dump_format(ofmt_ctx, 0, filename, 1);
    }
    else
    {
        av_dump_format(ofmt_ctx, 0, 0, 1);
    }
    ret = avformat_write_header(ofmt_ctx, &options);
    if(ret<0){
        fprintf(stderr, "Could not open write header");
        exit(1);
    }
}

void StreamHandler::record()
{
    qDebug() << "Starter record";
    if(mVideoEnabled)
    {
        QtConcurrent::run(videoHandler, &VideoHandler::grabFrames);
    }
    if(mAudioEnabled)
    {
        QtConcurrent::run(audioHandler, &AudioHandler::grabFrames);
    }

}

int StreamHandler::custom_io_write(void* opaque, uint8_t *buffer, int buffer_size)
{
    //qDebug() << "Inne i custom io write";
    SocketHandler* socketHandler = reinterpret_cast<SocketHandler*>(opaque);

    char *cptr = reinterpret_cast<char*>(const_cast<uint8_t*>(buffer));

    QByteArray send;
    send = QByteArray(reinterpret_cast<char*>(cptr), buffer_size);
    //qDebug() << "written to socket";

    //delete cptr;
    return socketHandler->sendDatagram(send);

    //outfile.write((char*)buffer, buffer_size);



    return 0;
}
