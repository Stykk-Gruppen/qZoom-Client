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
    int error = mVideoHandler->init();
    if(error<0)
    {
        fprintf(stderr, "Could not init videohandler");
        exit(1);
    }
/*
    error = audioHandler->init();
    if(error<0){
        fprintf(stderr, "Could not init audiohandler");
        exit(1);
    }
    */
}

void StreamHandler::record()
{
    /*
    qDebug() << "Starter record";
    if(mVideoEnabled)
    {
        QtConcurrent::run(mVideoHandler, &VideoHandler::grabFrames);
    }
    if(mAudioEnabled)
    {
        QtConcurrent::run(mAudioHandler, &AudioHandler::grabFrames);
    }
    */
}

void StreamHandler::enableAudio()
{
    mAudioEnabled = true;
    if (mAudioHandler == nullptr)
    {
        qDebug() << "creating mAudioHandler";
        int64_t time = av_gettime();
        mAudioHandler = new AudioHandler(mAudioDevice, &mUDPSendDatagramMutexLock, time, mSocketHandler);

        int error = mAudioHandler->init();
        if(error<0)
        {
            fprintf(stderr, "Could not init audiohandler");
            exit(1);
        }
    }

    mAudioHandler->toggleGrabFrames(mAudioEnabled);
    QtConcurrent::run(mAudioHandler, &AudioHandler::grabFrames);

}

void StreamHandler::disableAudio()
{
    mAudioEnabled = false;
    //delete mAudioHandler;
    mAudioHandler->toggleGrabFrames(mAudioEnabled);
}

void StreamHandler::enableVideo()
{
    mVideoEnabled = true;
    if (mVideoHandler == nullptr)
    {
        qDebug() << "new videohandler";
        int64_t time = av_gettime();
        mVideoHandler = new VideoHandler(mVideoDevice, &mUDPSendDatagramMutexLock, time, mImageHandler, mSocketHandler);

        int error = mVideoHandler->init();
        if(error < 0)
        {
            fprintf(stderr, "Could not init videohandler");
            exit(1);
        }
    }

    mVideoHandler->toggleGrabFrames(mVideoEnabled);
    QtConcurrent::run(mVideoHandler, &VideoHandler::grabFrames);
}

void StreamHandler::disableVideo()
{
    mVideoEnabled = false;
    //delete mVideoHandler;
    mVideoHandler->toggleGrabFrames(mVideoEnabled);
}

QVariantList StreamHandler::getAudioInputDevices()
{
    return mAudioHandler->getAudioInputDevices();
}

void StreamHandler::changeAudioInputDevice(QString deviceName)
{
    qDebug() << "Changing Audio to: " << deviceName;
    mAudioDevice = deviceName;
}

void StreamHandler::stopRecording()
{
    disableAudio();
    disableVideo();
}
