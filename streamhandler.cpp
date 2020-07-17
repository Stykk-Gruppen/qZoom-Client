#include "streamhandler.h"

StreamHandler::StreamHandler(ImageHandler* _imageHandler, SocketHandler* _socketHandler, QObject *parent) : QObject(parent)
{
    int64_t time = av_gettime();
    mVideoDevice = "/dev/video0";
    mAudioDevice = "default";
    mImageHandler = _imageHandler;
    mSocketHandler = _socketHandler;
    //mVideoHandler = new VideoHandler(mVideoDevice, &mUDPSendDatagramMutexLock, time, mImageHandler, mSocketHandler);
    //mAudioHandler = new AudioHandler(mAudioDevice, &mUDPSendDatagramMutexLock, time, mSocketHandler);
    /*
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
    */
}

void StreamHandler::record()
{
    qDebug() << "Starter record";
    if(mVideoEnabled)
    {
        QtConcurrent::run(mVideoHandler, &VideoHandler::grabFrames);
    }
    if(mAudioEnabled)
    {
        QtConcurrent::run(mAudioHandler, &AudioHandler::grabFrames);
    }
}

void StreamHandler::enableAudio()
{
    mAudioEnabled = true;
    if (1)
    {
        qDebug() << "creating mAudioHandler";
        int64_t time = av_gettime();
        mAudioHandler = new AudioHandler(mAudioDevice, &mUDPSendDatagramMutexLock, time, mSocketHandler);
    }
    int error = mAudioHandler->init();
    if(error<0){
        fprintf(stderr, "Could not init audiohandler");
        exit(1);
    }
}

void StreamHandler::disableAudio()
{
    mAudioEnabled = false;
    delete mAudioHandler;
}

void StreamHandler::enableVideo()
{
    mVideoEnabled = true;
    if (1)
    {
        int64_t time = av_gettime();
        mVideoHandler = new VideoHandler(mVideoDevice, &mUDPSendDatagramMutexLock, time, mImageHandler, mSocketHandler);
    }
    int error = mVideoHandler->init();
    if(error<0)
    {
        fprintf(stderr, "Could not init videohandler");
        exit(1);
    }
}

void StreamHandler::disableVideo()
{
    mVideoEnabled = false;
    delete mVideoHandler;
}

QVariantList StreamHandler::getAudioInputDevices()
{
    return mAudioHandler->getAudioInputDevices();
}

void StreamHandler::changeAudioInputDevice(QString deviceName)
{
    mAudioDevice = deviceName;
}

void StreamHandler::stopRecording()
{

}
