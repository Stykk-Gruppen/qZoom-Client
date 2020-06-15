#include "streamhandler.h"

StreamHandler::StreamHandler()
{

    ofmt_ctx = NULL;
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
    ofmt_ctx->oformat = av_guess_format(NULL, filename, NULL);


    videoHandler = new VideoHandler("/dev/video0", ofmt_ctx, &writeLock);
    audioHandler = new AudioHandler(ofmt_ctx, "fil.imsv");
    videoHandler->init();
}

void StreamHandler::record()
{
    QtConcurrent::run(videoHandler, &VideoHandler::grabFrames);

}
