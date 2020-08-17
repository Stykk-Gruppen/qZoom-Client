#include "sessionhandler.h"

/**
 * @brief SessionHandler::SessionHandler
 * @param _mServerTcpQueries
 * @param _user
 * @param imageHandler
 * @param settings
 * @param bufferSize
 * @param parent
 */
SessionHandler::SessionHandler(ServerTcpQueries* _mServerTcpQueries, UserHandler* _user,
                               ImageHandler* imageHandler,
                               Settings* settings, int bufferSize,
                               QObject *parent) : QObject(parent)
{
    mServerTcpQueries = _mServerTcpQueries;
    mUser = _user;
    setDefaultRoomID();
    mIpAddress = "Ipaddress";
    mSettings = settings;
    mBufferSize = bufferSize;
    mPortNumberTCP = settings->getTcpPort();
    mPortNumberUDP = settings->getUdpPort();
    mImageHandler = imageHandler;
    mSessionIsActive = false;

    mAddress = (mSettings->getServerIpAddress() == "Localhost") ?
                QHostAddress::LocalHost : QHostAddress(mSettings->getServerIpAddress());

}

/**
 * @brief SessionHandler::enableScreenShare
 * @return
 */
bool SessionHandler::enableScreenShare()
{
    if(mOutputStreamHandler->checkVideoEnabled())
    {
        mOutputStreamHandler->disableVideo();
    }
    return mOutputStreamHandler->enableVideo(true) >= 0;
}

/**
 * @brief SessionHandler::kickParticipant
 * @param index
 */
void SessionHandler::kickParticipant(const int &index) const
{
    const QString streamId = mInputStreamHandler->getStreamIdFromIndex(index);
    if (streamId.size() > 0)
    {
        mTcpSocketHandler->sendKickParticipantSignal(streamId);
    }
}

/**
 * @brief SessionHandler::isHost
 * @return
 */
bool SessionHandler::isHost() const
{
    return (mRoomHostUsername == mUser->getUsername() && !mUser->isGuest());
}

/**
 * @brief SessionHandler::enableVideo
 * @return
 */
bool SessionHandler::enableVideo()
{
    if (mOutputStreamHandler->checkVideoEnabled())
    {
        mOutputStreamHandler->disableVideo();
    }
    return mOutputStreamHandler->enableVideo() >= 0;
}

/**
 * @brief SessionHandler::disableVideo
 */
void SessionHandler::disableVideo()
{
    mOutputStreamHandler->disableVideo();
}

/**
 * @brief SessionHandler::enableAudio
 * @return
 */
bool SessionHandler::enableAudio()
{
    mImageHandler->setPeerAudioIsDisabled(std::numeric_limits<uint8_t>::max(), false);
    return mOutputStreamHandler->enableAudio() >= 0;
}

/**
 * @brief SessionHandler::disableAudio
 */
void SessionHandler::disableAudio()
{
    mOutputStreamHandler->disableAudio();
    mImageHandler->setPeerAudioIsDisabled(std::numeric_limits<uint8_t>::max(), true);
}

/**
 * @brief SessionHandler::initOtherStuff
 * @return
 */
int SessionHandler::initOtherStuff()
{
    const QString streamId = isGuest() ? mUser->getGuestStreamId() : mUser->getStreamId();
    const QString roomId = getRoomId();
    const QString displayName = mSettings->getDisplayName();
    mPortNumberUDP = mSettings->getUdpPort();
    mPortNumberTCP = mSettings->getTcpPort();
    mAddress = (mSettings->getServerIpAddress() == "Localhost") ? QHostAddress::LocalHost : QHostAddress(mSettings->getServerIpAddress());
    mSessionIsActive = true;
    mInputStreamHandler = new InputStreamHandler(mImageHandler, mBufferSize, mAddress);
    mUdpSocketHandler = new UdpSocketHandler(mBufferSize, mPortNumberUDP, mInputStreamHandler, streamId, roomId, mAddress);
    mTcpSocketHandler = new TcpSocketHandler(mInputStreamHandler, streamId, roomId, displayName, mAddress, mPortNumberTCP);
    connect(mTcpSocketHandler, &TcpSocketHandler::sendDummyDatagram, mUdpSocketHandler, &UdpSocketHandler::openPortHack);

    mOutputStreamHandler = new OutputStreamHandler(mImageHandler, mUdpSocketHandler, mBufferSize, mSettings, mTcpSocketHandler);

    if (mTcpSocketHandler->init() < 0)
    {
        return -1;
    }

    mOutputStreamHandler->init();
    return 0;
}

/**
 * @brief SessionHandler::deleteSocketHandlers
 */
void SessionHandler::deleteSocketHandlers()
{
    qDebug() << Q_FUNC_INFO;
    delete mUdpSocketHandler;
    delete mTcpSocketHandler;
}

/**
 * @brief SessionHandler::deleteStreams
 */
void SessionHandler::deleteStreams()
{
    delete mInputStreamHandler;
    delete mOutputStreamHandler;
}

/**
 * @brief SessionHandler::closeSocketHandlers
 */
void SessionHandler::closeSocketHandlers()
{
    mUdpSocketHandler->closeSocket();
    mTcpSocketHandler->close();
}

/**
 * @brief SessionHandler::closeStreams
 */
void SessionHandler::closeStreams()
{
    //This will clear all the vectors containing objects connected to each person in the room
    mInputStreamHandler->close();
    //This will close the output streams
    mOutputStreamHandler->close();
}

/**
 * @brief SessionHandler::getAudioInputDevices
 * @return
 */
QVariantList SessionHandler::getAudioInputDevices() const
{
    return AudioHandler::getAudioInputDevices();
}

/**
 * @brief SessionHandler::joinSession
 * @param roomId
 * @param roomPassword
 * @return
 */
bool SessionHandler::joinSession(const QString& roomId, const QString& roomPassword)
{
    qDebug() << "RoomId: " << roomId;
    QVariantList vars;
    vars.append(roomId);
    vars.append(roomPassword);
    const QVariantList response = mServerTcpQueries->serverQuery(0, vars);
    if (response.size() > 0)
    {
        if (response[0].toInt() == -1)
        {
            errorHandler->giveErrorDialog("Could not connect to server");
            return false;
        }

        mRoomId = response[0].toString();
        mRoomPassword = response[1].toString();
        qDebug() << "mRoomPassword: " << mRoomPassword;
        mRoomHostUsername = response[2].toString();
        addUser();
        mSettings->setLastRoomId(mRoomId);
        mSettings->setLastRoomPassword(mRoomPassword);
        mSettings->saveSettings();
        const uint8_t userIndex = std::numeric_limits<uint8_t>::max();
        mImageHandler->addPeer(userIndex, mSettings->getDisplayName());

        if (initOtherStuff() < 0)
        {
            leaveSession();
            errorHandler->giveErrorDialog("An unknown error occured when initializing stream");
            return false;
        }
        return true;
    }
    return false;
}

/**
 * @brief SessionHandler::getRoomId
 * @return
 */
QString SessionHandler::getRoomId() const
{
    return mRoomId;
}

/**
 * @brief SessionHandler::getRoomPassword
 * @return
 */
QString SessionHandler::getRoomPassword() const
{
    return mRoomPassword;
}

/**
 * @brief SessionHandler::addUser
 */
void SessionHandler::addUser()
{
    QVariantList vars;
    vars.append(mRoomId);

    qDebug() << "User is guest: " << mUser->isGuest();
    if (!mUser->isGuest())
    {
        vars.append(QString::number(mUser->getUserId()));
        const int numberOfRowsAffected = mServerTcpQueries->serverQuery(1, vars)[0].toInt();
        if(numberOfRowsAffected <= 0)
        {
            qDebug() << "Failed Query" << Q_FUNC_INFO;
        }
    }
    else
    {
        if (addGuestUserToDatabase())
        {
            vars.append(QString::number(mUser->getGuestId()));
            const int numberOfRowsAffected = mServerTcpQueries->serverQuery(1, vars)[0].toInt();
            if(numberOfRowsAffected <= 0)
            {
                qDebug() << "Failed Query" << Q_FUNC_INFO;
            }
        }
    }
}

/**
 * @brief SessionHandler::kickYourself
 */
void SessionHandler::kickYourself()
{
    mSessionIsActive = false;
    closeSocketHandlers();
    closeStreams();
    errorHandler->giveKickedErrorDialog();
}

/**
 * @brief SessionHandler::deleteStreamsAndSockets
 */
void SessionHandler::deleteStreamsAndSockets()
{
    deleteSocketHandlers();
    deleteStreams();
    mImageHandler->removeAllPeers();
}

/**
 * @brief SessionHandler::leaveSession
 * @return
 */
bool SessionHandler::leaveSession()
{
    mSessionIsActive = false;
    closeSocketHandlers();
    closeStreams();
    deleteSocketHandlers();
    deleteStreams();
    mImageHandler->removeAllPeers();
    return true;
}

/**
 * @brief SessionHandler::createSession
 * @param roomId
 * @param roomPassword
 * @return
 */
bool SessionHandler::createSession(const QString& roomId, const QString& roomPassword)
{
    if (!mUser->isGuest())
    {
        if (!mUser->hasRoom())
        {
            QVariantList vars;
            vars.append(roomPassword);
            vars.append(QString::number(mUser->getUserId()));
            vars.append(roomId);
            int numberOfRowsAffected = mServerTcpQueries->serverQuery(2, vars)[0].toInt();
            if(numberOfRowsAffected <= 0)
            {
                qDebug() << "Failed Query" << Q_FUNC_INFO;
                return false;
            }
            return true;
        }
        else
        {
            qDebug() << "User already has a room";
        }
    }
    return false;
}

/**
 * @brief SessionHandler::isGuest
 * @return
 */
bool SessionHandler::isGuest() const
{
    return mUser->isGuest();
}

/**
 * @brief SessionHandler::getRoomHostUsername
 * @return
 */
QString SessionHandler::getRoomHostUsername() const
{
    return mRoomHostUsername;
}

/**
 * @brief SessionHandler::addGuestUserToDatabase
 * @return
 */
bool SessionHandler::addGuestUserToDatabase()
{
    qDebug() << mUser->getGuestName();
    QVariantList vars;
    vars.append(mUser->getGuestName());
    int numberOfRowsAffected = mServerTcpQueries->serverQuery(3, vars)[0].toInt();
    if(numberOfRowsAffected <= 0)
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO;
        return false;
    }
    qDebug() << "Added guest :" << mUser->getGuestName() << "to the database";
    return true;
}

/**
 * @brief SessionHandler::getSessionIsActive
 * @return
 */
bool SessionHandler::getSessionIsActive() const
{
    return mSessionIsActive;
}

/**
 * @brief SessionHandler::setDefaultRoomID
 */
void SessionHandler::setDefaultRoomID()
{
    mRoomId = "Debug";
}

/**
 * @brief SessionHandler::updateDisplayName
 */
void SessionHandler::updateDisplayName()
{
    qDebug() << "SessionHandler will updateDisplayName";
    if (mSessionIsActive)
    {
        qDebug() << "Session is active";
        mImageHandler->updatePeerDisplayName(std::numeric_limits<uint8_t>::max(), mSettings->getDisplayName());
        mTcpSocketHandler->updateDisplayName(mSettings->getDisplayName());
        mTcpSocketHandler->sendChangedDisplayNameSignal();
    }
}

/**
 * @brief SessionHandler::checkVideoEnabled
 * @return
 */
bool SessionHandler::checkVideoEnabled() const
{
    return mOutputStreamHandler->checkVideoEnabled();
}

/**
 * @brief SessionHandler::checkAudioEnabled
 * @return
 */
bool SessionHandler::checkAudioEnabled() const
{
    return mOutputStreamHandler->checkAudioEnabled();
}
