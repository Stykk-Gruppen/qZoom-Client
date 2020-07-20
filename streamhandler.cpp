#include "streamhandler.h"

StreamHandler::StreamHandler(ImageHandler* _imageHandler, SocketHandler* socketHandler, int bufferSize, QObject *parent) : QObject(parent)
{
    mBufferSize = bufferSize;
    int64_t time = av_gettime();
    videoHandler = new VideoHandler("/dev/video0", &mUDPSendDatagramMutexLock, time, _imageHandler,socketHandler, mBufferSize);
    audioHandler = new AudioHandler("default", &mUDPSendDatagramMutexLock,time, socketHandler, mBufferSize);
    int error = videoHandler->init();
    if(error<0)
    {
        fprintf(stderr, "Could not init videohandler");
        exit(1);
    }

    error = audioHandler->init();
    if(error<0){
        fprintf(stderr, "Could not init audiohandler");
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

