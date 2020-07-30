#include "streamhandler.h"


StreamHandler::StreamHandler(ImageHandler* _imageHandler, SocketHandler* _socketHandler, int bufferSize, Settings* settings, TcpSocketHandler* tcpSocketHandler,  QObject *parent) : QObject(parent)
{
    mSettings = settings;
    mBufferSize = bufferSize;

    mTime = av_gettime();
    mVideoDevice = "/dev/video0";
    mAudioDevice = settings->getDefaultAudioInput();
    mImageHandler = _imageHandler;
    mSocketHandler = _socketHandler;
    mTcpSocketHandler = tcpSocketHandler;
    if(!mVideoEnabled) mImageHandler->readImage(nullptr, nullptr, 0);
}

void StreamHandler::init()
{
    mVideoEnabled = mSettings->getVideoOn();
    mAudioEnabled = mSettings->getAudioOn();
    //Setter opp video
    mTcpSocketHandler->init();

    if(mVideoEnabled)
    {
        if(enableVideo() < 0)
        {

            mTcpSocketHandler->writeHeader();
            mVideoEnabled = false;
        }
    }
    //Skriver tom header hvis video ikke er enabled, og skriver full header hvis den er enabled;
    else
    {
        mTcpSocketHandler->writeHeader();
        mVideoEnabled = false;

    }

    if(mAudioEnabled)
    {
        if(enableAudio() >= 0)
            return;
    }
    mAudioEnabled = false;
}

void StreamHandler::close()
{
    delete mAudioHandler;
    delete mVideoHandler;
}



void StreamHandler::grabVideoHeader()
{
    if(mVideoHandler == nullptr)
    {
        qDebug() << "Creating new VideoHandler";
        mVideoHandler = new VideoHandler(mVideoDevice, &mUDPSendDatagramMutexLock,
                                                             mTime, mImageHandler, mSocketHandler,mBufferSize, mTcpSocketHandler);
    }
    qDebug() << "Init videoHandler";
    mVideoHandler->init();
    qDebug() << "Close VideoHandler";
    mVideoHandler->close();
    qDebug() << "Sending generic image to imagehandler";
    mImageHandler->readImage(nullptr, nullptr, 0);
    delete mVideoHandler;
    mVideoHandler = nullptr;
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

int StreamHandler::enableAudio()
{
    mAudioEnabled = true;
    qDebug() << "enabling audio";
    if (mAudioHandler == nullptr)
    {
        qDebug() << "creating mAudioHandler";
        //int64_t time = av_gettime();
        mAudioHandler = new AudioHandler(mAudioDevice, &mUDPSendDatagramMutexLock,
                                         mTime, mSocketHandler,mBufferSize);
    }

    int error = mAudioHandler->init();
    if(error<0)
    {
        fprintf(stderr, "Could not init audiohandler");
        errorHandler->giveErrorDialog("Could not stream audio");

        return error;
    }


    mAudioHandler->toggleGrabFrames(mAudioEnabled);
    QtConcurrent::run(mAudioHandler, &AudioHandler::grabFrames);
    return 0;
}

void StreamHandler::disableAudio()
{
    if(mAudioHandler != nullptr)
    {
        mAudioEnabled = false;
        mAudioHandler->toggleGrabFrames(mAudioEnabled);
        //delete mAudioHandler;
        qDebug() << "audio disabled";
    }
}

int StreamHandler::enableVideo()
{
    mVideoEnabled = true;
    qDebug() << "enabling video";
    if (mVideoHandler == nullptr)
    {
        qDebug() << "new videohandler";
        //int64_t time = av_gettime();
        mVideoHandler = new VideoHandler(mVideoDevice, &mUDPSendDatagramMutexLock,
                                         mTime, mImageHandler, mSocketHandler,mBufferSize, mTcpSocketHandler);
    }

    int error = mVideoHandler->init();
    mTcpSocketHandler->writeHeader();
    if(error < 0)
    {
        fprintf(stderr, "Could not init videohandler");
        errorHandler->giveErrorDialog("Could not stream video");
        return error;
    }

    mVideoHandler->toggleGrabFrames(mVideoEnabled);
    qDebug() << "Before grab frames";
    QtConcurrent::run(mVideoHandler, &VideoHandler::grabFrames);
    qDebug() << "Grab frames started";
    return 0;
}

void StreamHandler::disableVideo()
{
    if(mVideoHandler != nullptr)
    {
        mVideoEnabled = false;
        //delete mVideoHandler;
        mVideoHandler->toggleGrabFrames(mVideoEnabled);
        qDebug() << "video disabled";
    }
}

QVariantList StreamHandler::getAudioInputDevices()
{
    return mAudioHandler->getAudioInputDevices();
}

void StreamHandler::changeAudioInputDevice(QString deviceName)
{
    qDebug() << "Changing Audio to: " << deviceName;
    mAudioDevice = deviceName;
    mAudioHandler->changeAudioInputDevice(mAudioDevice);
    disableAudio();
    enableAudio();
}

QString StreamHandler::getDefaultAudioInputDevice()
{
    return "default";
}

void StreamHandler::stopRecording()
{
    disableAudio();
    disableVideo();
}
