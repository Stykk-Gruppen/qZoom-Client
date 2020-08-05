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

UserHandler* SessionHandler::getUser()
{
    return mUser;
}

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
    mImageHandler->setPeerAudioIsDisabled(255, false);
    return mOutputStreamHandler->enableAudio() >= 0;
}

void SessionHandler::disableAudio()
{
    mOutputStreamHandler->disableAudio();
    mImageHandler->setPeerAudioIsDisabled(255, true);
}

int SessionHandler::initOtherStuff()
{
    QString streamId = (isGuest()) ? getUser()->getGuestStreamId() : getUser()->getStreamId();
    QString roomId = getRoomId();
    QString displayName = mSettings->getDisplayName();
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

QVariantList SessionHandler::getAudioInputDevices()
{
    return AudioHandler::getAudioInputDevices();

}

bool SessionHandler::joinSession(QString _roomId, QString _roomPassword)
{
    qDebug() << "RoomId: " << _roomId;
    QVariantList response = mServerTcpQueries->querySelectFrom_room1(_roomId,_roomPassword);
    if(response.size()>0)
    {
        mRoomId = response[0].toString();
        mRoomPassword = response[1].toString();
        qDebug() << "mRoomPassword: " << mRoomPassword;
        mRoomHostUsername = response[2].toString();
        addUser();
        mSettings->setLastRoomId(mRoomId);
        mSettings->setLastRoomPassword(mRoomPassword);
        mSettings->saveSettings();
        uint8_t userIndex = std::numeric_limits<uint8_t>::max();
        mImageHandler->addPeer(userIndex, mSettings->getDisplayName());
        if(initOtherStuff() < 0)
        {
            closeOtherStuff();
            return false;
        }
        return true;
    }
    return false;

}

QString SessionHandler::getRoomId()
{
    return mRoomId;
}

QString SessionHandler::getRoomPassword()
{
    return mRoomPassword;
}

void SessionHandler::addUser()
{
    qDebug() << "User is guest: " << mUser->isGuest();
    if (!mUser->isGuest())
    {
        int numberOfRowsAffected = mServerTcpQueries->queryInsertInto_roomSession(mRoomId, QString::number(mUser->getUserId()));
        if(numberOfRowsAffected<=0)
        {
            qDebug() << "Failed Query" << Q_FUNC_INFO;
        }
    }
    else
    {
        if (addGuestUserToDatabase())
        {
            int numberOfRowsAffected = mServerTcpQueries->queryInsertInto_roomSession(mRoomId, QString::number(mUser->getGuestId()));
            if(numberOfRowsAffected<=0)
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
}

bool SessionHandler::createSession(QString roomId, QString roomPassword)
{
    if (!mUser->isGuest())
    {
        if (!mUser->hasRoom())
        {
            int numberOfRowsAffected = mServerTcpQueries->queryInsertInto_room(roomId, QString::number(mUser->getUserId()),roomPassword);
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

bool SessionHandler::isGuest()
{
    return mUser->isGuest();
}

QString SessionHandler::getRoomHostUsername()
{
    return mRoomHostUsername;
}

bool SessionHandler::addGuestUserToDatabase()
{
    qDebug() << mUser->getGuestName();
    int numberOfRowsAffected = mServerTcpQueries->queryInsertInto_user(mUser->getGuestName());
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

bool SessionHandler::checkVideoEnabled()
{
    return mOutputStreamHandler->checkVideoEnabled();
}

bool SessionHandler::checkAudioEnabled()
{
    return mOutputStreamHandler->checkAudioEnabled();
}
