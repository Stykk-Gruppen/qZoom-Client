#include "sessionhandler.h"

SessionHandler::SessionHandler(ServerTcpQueries* _mServerTcpQueries, UserHandler* _user,
                               ImageHandler* imageHandler,
                               Settings* settings, int bufferSize,
                               QHostAddress address, int _portNumberTCP,
                               int _portNumerUDP,
                               QObject *parent) : QObject(parent)
{
    mServerTcpQueries = _mServerTcpQueries;
    mUser = _user;
    setDefaultRoomID();
    mIpAddress = "Ipaddress";
    mSettings = settings;
    mBufferSize = bufferSize;
    mPortNumberTCP = _portNumberTCP;
    mPortNumberUDP = _portNumerUDP;
    mAddress = address;
    mImageHandler = imageHandler;
    mSessionIsActive = false;
}

/*
UserHandler* SessionHandler::getUser() const
{
    return mUser;
}
*/

bool SessionHandler::enableScreenShare()
{
    if(mOutputStreamHandler->checkVideoEnabled())
    {
        mOutputStreamHandler->disableVideo();
    }
    return mOutputStreamHandler->enableVideo(true) >= 0;
}

bool SessionHandler::enableVideo()
{
    if(mOutputStreamHandler->checkVideoEnabled())
    {
        mOutputStreamHandler->disableVideo();
    }
    return mOutputStreamHandler->enableVideo() >= 0;
}

void SessionHandler::disableVideo()
{
    mOutputStreamHandler->disableVideo();
}

bool SessionHandler::enableAudio()
{
    mImageHandler->setPeerAudioIsDisabled(std::numeric_limits<uint8_t>::max(), false);
    return mOutputStreamHandler->enableAudio() >= 0;
}

void SessionHandler::disableAudio()
{
    mOutputStreamHandler->disableAudio();
    mImageHandler->setPeerAudioIsDisabled(std::numeric_limits<uint8_t>::max(), true);
}

int SessionHandler::initOtherStuff()
{
    const QString streamId = isGuest() ? mUser->getGuestStreamId() : mUser->getStreamId();
    const QString roomId = getRoomId();
    const QString displayName = mSettings->getDisplayName();
    mSessionIsActive = true;
    mInputStreamHandler = new InputStreamHandler(mImageHandler, mBufferSize, mAddress);
    mUdpSocketHandler = new UdpSocketHandler(mBufferSize, mPortNumberUDP, mInputStreamHandler, streamId, roomId, mAddress);
    //mTcpServerHandler = new TcpServerHandler(mInputStreamHandler, mPort);
    mTcpSocketHandler = new TcpSocketHandler(mInputStreamHandler, streamId, roomId, displayName, mAddress, mPortNumberTCP);
    mOutputStreamHandler = new OutputStreamHandler(mImageHandler, mUdpSocketHandler, mBufferSize, mSettings, mTcpSocketHandler);
    //Init tcpServerHandler
    //mTcpServerHandler->init();
    //Init sending of our header, empty or not
    if(mTcpSocketHandler->init() < 0)
    {
        return -1;
    }

    mOutputStreamHandler->init();
    return 0;
}

void SessionHandler::closeOtherStuff()
{
    mSessionIsActive = false;
    qDebug() << "Before close";
    //Sockets should close first, they use buffers and locks inside inputStreamHandler
    mUdpSocketHandler->closeSocket();
    mTcpSocketHandler->close();
    //This will clear all the vectors containing objects connected to each person in the room
    mInputStreamHandler->close();
    //This will close the output streams
    mOutputStreamHandler->close();

    qDebug() << "After close, about to delete";
    delete mUdpSocketHandler;
    delete mTcpSocketHandler;
    delete mInputStreamHandler;
    delete mOutputStreamHandler;
    qDebug() << "Deleted everything";
    mImageHandler->removeAllPeers();
}

QVariantList SessionHandler::getAudioInputDevices() const
{
    return AudioHandler::getAudioInputDevices();
}

bool SessionHandler::joinSession(QString _roomId, QString _roomPassword)
{
    qDebug() << "RoomId: " << _roomId;
    QVariantList vars;
    vars.append(_roomPassword);
    vars.append(_roomId);
    const QVariantList response = mServerTcpQueries->RQuery(0, vars);
    if(response.size() > 0)
    {
        if(response[0].toInt() == -1)
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

        if(initOtherStuff() < 0)
        {
            closeOtherStuff();
            errorHandler->giveErrorDialog("An unknown error occured when initializing stream");
            return false;
        }
        return true;
    }
    return false;
}

QString SessionHandler::getRoomId() const
{
    return mRoomId;
}

QString SessionHandler::getRoomPassword() const
{
    return mRoomPassword;
}

void SessionHandler::addUser()
{
    QVariantList vars;
    vars.append(mRoomId);

    qDebug() << "User is guest: " << mUser->isGuest();
    if (!mUser->isGuest())
    {
        vars.prepend(QString::number(mUser->getUserId()));
        const int numberOfRowsAffected = mServerTcpQueries->CUDQuery(1, vars);
        if(numberOfRowsAffected <= 0)
        {
            qDebug() << "Failed Query" << Q_FUNC_INFO;
        }
    }
    else
    {
        if (addGuestUserToDatabase())
        {
            vars.prepend(QString::number(mUser->getGuestId()));
            const int numberOfRowsAffected = mServerTcpQueries->CUDQuery(1, vars);
            if(numberOfRowsAffected <= 0)
            {
                qDebug() << "Failed Query" << Q_FUNC_INFO;
            }
        }
    }
}

bool SessionHandler::leaveSession()
{
    //CLose all that is opened;
    closeOtherStuff();
    return true;
}

bool SessionHandler::createSession(QString roomId, QString roomPassword)
{
    if (!mUser->isGuest())
    {
        if (!mUser->hasRoom())
        {
            QVariantList vars;
            vars.append(roomId);
            vars.append(QString::number(mUser->getUserId()));
            vars.append(roomPassword);
            int numberOfRowsAffected = mServerTcpQueries->CUDQuery(2, vars);
            if(numberOfRowsAffected<=0)
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

bool SessionHandler::isGuest() const
{
    return mUser->isGuest();
}

QString SessionHandler::getRoomHostUsername() const
{
    return mRoomHostUsername;
}

bool SessionHandler::addGuestUserToDatabase()
{
    qDebug() << mUser->getGuestName();
    QVariantList vars;
    vars.append(mUser->getGuestName());
    int numberOfRowsAffected = mServerTcpQueries->CUDQuery(3, vars);
    if(numberOfRowsAffected<=0)
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO;
        return false;
    }
    qDebug() << "Added guest :" << mUser->getGuestName() << "to the database";
    return true;
}

void SessionHandler::setDefaultRoomID()
{
    mRoomId = "Debug";
}

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

bool SessionHandler::checkVideoEnabled() const
{
    return mOutputStreamHandler->checkVideoEnabled();
}

bool SessionHandler::checkAudioEnabled() const
{
    return mOutputStreamHandler->checkAudioEnabled();
}
