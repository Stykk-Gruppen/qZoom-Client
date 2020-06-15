#include "streamhandler.h"

StreamHandler::StreamHandler()
{
    avcodec_register_all();
    av_register_all();
    int ret;
    ofmt_ctx = NULL;

    socketHandler = new SocketHandler();
    socketHandler->initSocket();

    if(writeToFile)
    {
        avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);

    }
    else
    {
        ofmt_ctx = avformat_alloc_context();
    }
    ofmt_ctx->oformat = av_guess_format(NULL, filename, NULL);

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
        int avio_buffer_size = 4 * 1024;
        void* avio_buffer = av_malloc(avio_buffer_size);
        AVIOContext* custom_io = avio_alloc_context (
                    (unsigned char*)avio_buffer, avio_buffer_size,
                    1,
                    (void*) socketHandler,
                    NULL, &custom_io_write, NULL);
        ofmt_ctx->pb = custom_io;
        av_dump_format(ofmt_ctx, 0, filename, 1);
        av_dict_set(&options, "live", "1", 0);
    }
    qDebug() << "Kom hit!";
    videoHandler = new VideoHandler("/dev/video0", ofmt_ctx, writeToFile, &writeLock, numberOfFrames);
    audioHandler = new AudioHandler(ofmt_ctx, "fil.imsv");
    videoHandler->init();

    ret = avformat_write_header(ofmt_ctx, &options);
}

void StreamHandler::record()
{
    QtConcurrent::run(videoHandler, &VideoHandler::grabFrames);

}

int StreamHandler::custom_io_write(void* opaque, uint8_t *buffer, int buffer_size)
{
    SocketHandler* socketHandler = reinterpret_cast<SocketHandler*>(opaque);

    char *cptr = reinterpret_cast<char*>(const_cast<uint8_t*>(buffer));

    QByteArray send;
    send = QByteArray(reinterpret_cast<char*>(cptr), buffer_size);
    return socketHandler->sendDatagram(send);

    //outfile.write((char*)buffer, buffer_size);
}
