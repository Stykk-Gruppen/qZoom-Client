#include "streamhandler.h"

StreamHandler::StreamHandler(ImageHandler* _imageHandler, SocketHandler* socketHandler, QObject *parent) : QObject(parent)
{
    int64_t time = av_gettime();
    videoHandler = new VideoHandler("/dev/video0", &mUDPSendDatagramMutexLock, time, _imageHandler,socketHandler);
    audioHandler = new AudioHandler("default", &mUDPSendDatagramMutexLock,time, socketHandler);
    int ret = videoHandler->init();
    if(ret<0)
    {
        fprintf(stderr, "Could not init videohandler");
        exit(1);
    }
    ret = audioHandler->init();
    if(ret<0){
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
