#include "streamhandler.h"


StreamHandler::StreamHandler(ImageHandler* _imageHandler, SocketHandler* _socketHandler, int bufferSize, QObject *parent) : QObject(parent)
{
    mBufferSize = bufferSize;
    mTime = av_gettime();
    mVideoDevice = "/dev/video0";
    mAudioDevice = "default";
    mImageHandler = _imageHandler;
    mSocketHandler = _socketHandler;

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
    qDebug() << "enabling audio";
    if (mAudioHandler == nullptr)
    {
        qDebug() << "creating mAudioHandler";
        //int64_t time = av_gettime();
        mAudioHandler = new AudioHandler(mAudioDevice, &mUDPSendDatagramMutexLock,
                                         mTime, mSocketHandler,mBufferSize);

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

    mAudioHandler->toggleGrabFrames(mAudioEnabled);
    //delete mAudioHandler;
    qDebug() << "audio disabled";
}

void StreamHandler::enableVideo()
{
    mVideoEnabled = true;
    qDebug() << "enabling video";
    if (mVideoHandler == nullptr)
    {
        qDebug() << "new videohandler";
        //int64_t time = av_gettime();
        mVideoHandler = new VideoHandler(mVideoDevice, &mUDPSendDatagramMutexLock,
                                         mTime, mImageHandler, mSocketHandler,mBufferSize);
    }

    int error = mVideoHandler->init();
    if(error < 0)
    {
        fprintf(stderr, "Could not init videohandler");
        exit(1);
    }

    mVideoHandler->toggleGrabFrames(mVideoEnabled);

    QtConcurrent::run(mVideoHandler, &VideoHandler::grabFrames);
}

void StreamHandler::disableVideo()
{
    mVideoEnabled = false;
    //delete mVideoHandler;
    mVideoHandler->toggleGrabFrames(mVideoEnabled);
    qDebug() << "video disabled";
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
