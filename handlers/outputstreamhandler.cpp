#include "outputstreamhandler.h"

/**
 * @brief OutputStreamHandler::OutputStreamHandler
 * @param _imageHandler
 * @param _socketHandler
 * @param bufferSize
 * @param settings
 * @param tcpSocketHandler
 * @param parent
 */
OutputStreamHandler::OutputStreamHandler(ImageHandler* _imageHandler, UdpSocketHandler* _socketHandler,
                                         size_t bufferSize, Settings* settings,
                                         TcpSocketHandler* tcpSocketHandler,  QObject *parent) : QObject(parent)
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
        mImageHandler->readImage(nullptr, nullptr, std::numeric_limits<uint8_t>::max());
    }
}

/**
 * @brief OutputStreamHandler::init
 */
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
            mTcpSocketHandler->sendVideoHeader();
            mVideoEnabled = false;
        }
    }
    else
    {
        //Skriver tom header hvis video ikke er enabled, og skriver full header hvis den er enabled;
        mTcpSocketHandler->sendVideoHeader();
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

/**
 * @brief OutputStreamHandler::close
 */
void OutputStreamHandler::close()
{
    qDebug() << "Closing outputStreamHandler";

    if(mAudioHandler != nullptr)
    {
        disableAudio();
    }

    if(mVideoHandler != nullptr)
    {
        disableVideo();
    }
}

/**
 * @brief OutputStreamHandler::grabVideoHeader
 */
void OutputStreamHandler::grabVideoHeader()
{
    if(mVideoHandler == nullptr)
    {
        qDebug() << "Creating new VideoHandler";
        mVideoHandler = new VideoHandler(mVideoDevice, &mUDPSendDatagramMutexLock,
                                         mTime, mImageHandler, mSocketHandler,
                                         mBufferSize, mTcpSocketHandler, false);
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

/**
 * @brief OutputStreamHandler::enableAudio
 * @return
 */
int OutputStreamHandler::enableAudio()
{
    mAudioEnabled = true;
    qDebug() << "enabling audio";
    if (mAudioHandler == nullptr)
    {
        qDebug() << "creating mAudioHandler";
        mAudioHandler = new AudioHandler(mAudioDevice, &mUDPSendDatagramMutexLock,
                                         mTime, mSocketHandler, mBufferSize, mImageHandler);
    }

    const int error = mAudioHandler->init();
    if(error < 0)
    {
        fprintf(stderr, "Could not init audiohandler");
        errorHandler->giveErrorDialog("Could not stream audio");
        return error;
    }

    mAudioHandler->toggleGrabFrames(mAudioEnabled);
    mAudioFuture = QtConcurrent::run(mAudioHandler, &AudioHandler::grabFrames);
    return 0;
}

/**
 * @brief OutputStreamHandler::disableAudio
 */
void OutputStreamHandler::disableAudio()
{
    if(mAudioHandler != nullptr)
    {
        mAudioEnabled = false;
        mAudioHandler->toggleGrabFrames(mAudioEnabled);
        mAudioFuture.waitForFinished();

        qDebug() << "Audiohandler not active, deleting it";
        delete mAudioHandler;
        mAudioHandler = nullptr;
        qDebug() << "audio disabled";

        if(mTcpSocketHandler->isOpen())
        {
            mTcpSocketHandler->sendDisabledAudioSignal();
        }
    }
}

/**
 * @brief OutputStreamHandler::enableVideo
 * @param screenShare
 * @return
 */
int OutputStreamHandler::enableVideo(bool screenShare)
{
    mVideoEnabled = true;
    qDebug() << "enabling video";
    if (mVideoHandler == nullptr)
    {
        qDebug() << "new videohandler";
        mVideoHandler = new VideoHandler(mVideoDevice, &mUDPSendDatagramMutexLock,
                                         mTime, mImageHandler, mSocketHandler,
                                         mBufferSize, mTcpSocketHandler, screenShare);
    }

    const int error = mVideoHandler->init();
    if(error < 0)
    {
        fprintf(stderr, "Could not init videohandler");
        errorHandler->giveErrorDialog("Could not stream video");
        qDebug() << "Error: " << error;
        return (int)error;
    }

    mVideoHandler->toggleGrabFrames(mVideoEnabled);
    qDebug() << "Before grab frames";
    mVideoFuture = QtConcurrent::run(mVideoHandler, &VideoHandler::grabFrames);
    qDebug() << "Grab frames started";
    return 0;
}

/**
 * @brief OutputStreamHandler::disableVideo
 */
void OutputStreamHandler::disableVideo()
{
    if(mVideoHandler != nullptr)
    {
        mVideoEnabled = false;
        mVideoHandler->toggleGrabFrames(mVideoEnabled);
        qDebug() << "video disabled";

        mVideoFuture.waitForFinished();
        qDebug() << "Videohandler not active, deleting it";

        delete mVideoHandler;
        mVideoHandler = nullptr;

        if(mTcpSocketHandler->isOpen())
        {
            mTcpSocketHandler->sendDisabledVideoSignal();
        }
    }
}

/**
 * @brief OutputStreamHandler::getAudioInputDevices
 * @return
 */
QVariantList OutputStreamHandler::getAudioInputDevices() const
{
    return AudioHandler::getAudioInputDevices();
}

/**
 * @brief OutputStreamHandler::changeAudioInputDevice
 * @param deviceName
 */
void OutputStreamHandler::changeAudioInputDevice(const QString& deviceName)
{
    qDebug() << "Changing Audio to: " << deviceName;
    mAudioDevice = deviceName;
    mAudioHandler->changeAudioInputDevice(mAudioDevice);
    disableAudio();
    enableAudio();
}

/**
 * @brief OutputStreamHandler::getDefaultAudioInputDevice
 * @return
 */
QString OutputStreamHandler::getDefaultAudioInputDevice() const
{
    return "default";
}

/**
 * @brief OutputStreamHandler::checkVideoEnabled
 * @return
 */
bool OutputStreamHandler::checkVideoEnabled() const
{
    return mVideoEnabled;
}

/**
 * @brief OutputStreamHandler::checkAudioEnabled
 * @return
 */
bool OutputStreamHandler::checkAudioEnabled() const
{
    return mAudioEnabled;
}
