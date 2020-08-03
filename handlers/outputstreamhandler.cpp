#include "outputstreamhandler.h"


OutputStreamHandler::OutputStreamHandler(ImageHandler* _imageHandler, UdpSocketHandler* _socketHandler, size_t bufferSize, Settings* settings, TcpSocketHandler* tcpSocketHandler,  QObject *parent) : QObject(parent)
{
    mSettings = settings;
    mBufferSize = bufferSize;

    mTime = av_gettime();
    mVideoDevice = "/dev/video0";
    mAudioDevice = settings->getDefaultAudioInput();
    mImageHandler = _imageHandler;
    mSocketHandler = _socketHandler;
    mTcpSocketHandler = tcpSocketHandler;
    if(!mVideoEnabled)
    {
        mImageHandler->readImage(nullptr, nullptr, 0);
    }
}

void OutputStreamHandler::init()
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
    else
    {
        //Skriver tom header hvis video ikke er enabled, og skriver full header hvis den er enabled;
        mTcpSocketHandler->writeHeader();
        mVideoEnabled = false;

    }

    if(mAudioEnabled)
    {
        if(enableAudio() < 0)
        {
           mAudioEnabled = false;
        }
    }
}

void OutputStreamHandler::close()
{
    qDebug() << "Closing outputStreamHandler";

    if(mAudioHandler != nullptr)
    {
        disableAudio();
        while(mAudioHandler->isActive())
        {
            av_usleep(500);
            qDebug() << "Audio handler is still active";
        }
        qDebug() << "Audiohandler not active, deleting it";
        delete mAudioHandler;

    }

    if(mVideoHandler != nullptr)
    {
        disableVideo();
        qDebug() << "After disable video";
        while(mVideoHandler->isActive())
        {
            av_usleep(500);
            qDebug() << "VideoHandler is still active";
        }
        qDebug() << "Videohandler not active, deleting it";

        delete mVideoHandler;
    }
}



void OutputStreamHandler::grabVideoHeader()
{
    if(mVideoHandler == nullptr)
    {
        qDebug() << "Creating new VideoHandler";
        mVideoHandler = new VideoHandler(mVideoDevice, &mUDPSendDatagramMutexLock,
                                         mTime, mImageHandler, mSocketHandler,
                                         mBufferSize, mTcpSocketHandler);
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

int OutputStreamHandler::enableAudio()
{
    mAudioEnabled = true;
    qDebug() << "enabling audio";
    if (mAudioHandler == nullptr)
    {
        qDebug() << "creating mAudioHandler";
        //int64_t time = av_gettime();
        mAudioHandler = new AudioHandler(mAudioDevice, &mUDPSendDatagramMutexLock,
                                         mTime, mSocketHandler, mBufferSize);
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

void OutputStreamHandler::disableAudio()
{
    if(mAudioHandler != nullptr)
    {
        mAudioEnabled = false;
        mAudioHandler->toggleGrabFrames(mAudioEnabled);
        //delete mAudioHandler;
        qDebug() << "audio disabled";
        if(mTcpSocketHandler->isOpen())
        {
            mTcpSocketHandler->sendDisabledAudioSignal();
        }
    }
}

int OutputStreamHandler::enableVideo()
{
    mVideoEnabled = true;
    qDebug() << "enabling video";
    if (mVideoHandler == nullptr)
    {
        qDebug() << "new videohandler";
        //int64_t time = av_gettime();
        mVideoHandler = new VideoHandler(mVideoDevice, &mUDPSendDatagramMutexLock,
                                         mTime, mImageHandler, mSocketHandler,
                                         mBufferSize, mTcpSocketHandler);
    }

    int error = mVideoHandler->init();
    mTcpSocketHandler->writeHeader();
    if(error < 0)
    {
        fprintf(stderr, "Could not init videohandler");
        errorHandler->giveErrorDialog("Could not stream video");
        qDebug() << "Error: " << error;
        return (int)error;
    }

    mVideoHandler->toggleGrabFrames(mVideoEnabled);
    qDebug() << "Before grab frames";
    QtConcurrent::run(mVideoHandler, &VideoHandler::grabFrames);
    qDebug() << "Grab frames started";
    return 0;
}

void OutputStreamHandler::disableVideo()
{
    if(mVideoHandler != nullptr)
    {
        mVideoEnabled = false;
        //delete mVideoHandler;
        mVideoHandler->toggleGrabFrames(mVideoEnabled);
        qDebug() << "video disabled";
        if(mTcpSocketHandler->isOpen())
        {
            mTcpSocketHandler->sendDisabledVideoSignal();
        }
    }
}

QVariantList OutputStreamHandler::getAudioInputDevices()
{
    return mAudioHandler->getAudioInputDevices();
}

void OutputStreamHandler::changeAudioInputDevice(QString deviceName)
{
    qDebug() << "Changing Audio to: " << deviceName;
    mAudioDevice = deviceName;
    mAudioHandler->changeAudioInputDevice(mAudioDevice);
    disableAudio();
    enableAudio();
}

QString OutputStreamHandler::getDefaultAudioInputDevice()
{
    return "default";
}

bool OutputStreamHandler::checkVideoEnabled()
{
    return mVideoEnabled;
}

bool OutputStreamHandler::checkAudioEnabled()
{
    return mAudioEnabled;
}