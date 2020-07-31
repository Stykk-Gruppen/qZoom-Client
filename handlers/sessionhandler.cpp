#include "sessionhandler.h"

SessionHandler::SessionHandler(Database* _db, UserHandler* _user,
                               ImageHandler* imageHandler,
                               Settings* settings, int bufferSize,
                               QHostAddress address, int port,
                               QObject *parent) : QObject(parent)
{
    mDb = _db;
    mUser = _user;
    setDefaultRoomID();
    mIpAddress = "Ipaddress";
    mSettings = settings;
    mBufferSize = bufferSize;
    mPort = port;
    mAddress = address;
    mImageHandler = imageHandler;
    mSessionIsActive = false;
}

UserHandler* SessionHandler::getUser()
{
    return mUser;
}

bool SessionHandler::enableVideo()
{
   return mStreamHandler->enableVideo() >= 0;

}

void SessionHandler::disableVideo()
{
    mStreamHandler->disableVideo();
}

bool SessionHandler::enableAudio()
{
    return mStreamHandler->enableAudio() >= 0;
}

void SessionHandler::disableAudio()
{
    mStreamHandler->disableAudio();
}

void SessionHandler::initOtherStuff()
{
    QString streamId = (isGuest()) ? getUser()->getGuestName() : getUser()->getStreamId();
    QString roomId = getRoomId();
    QString displayName = mSettings->getDisplayName();
    mSessionIsActive = true;
    mInputStreamHandler = new InputStreamHandler(mImageHandler, mBufferSize, mAddress);
    mSocketHandler = new SocketHandler(mBufferSize, mPort, mInputStreamHandler, streamId, roomId, mAddress);
    //mTcpServerHandler = new TcpServerHandler(mInputStreamHandler, mPort);
    mTcpSocketHandler = new TcpSocketHandler(mInputStreamHandler, streamId, roomId, displayName, mAddress, mPort);
    mStreamHandler = new StreamHandler(mImageHandler, mSocketHandler, mBufferSize, mSettings, mTcpSocketHandler);
    //Init tcpServerHandler
    //mTcpServerHandler->init();
    //Init sending of our header, empty or not
    mTcpSocketHandler->init();
    mStreamHandler->init();
}

void SessionHandler::closeOtherStuff()
{
    mSessionIsActive = false;
    qDebug() << "Before close";
    mInputStreamHandler->close();
    mSocketHandler->closeSocket();
    mTcpSocketHandler->close();
    mStreamHandler->close();
    qDebug() << "After close, about to delete";
    delete mInputStreamHandler;
    delete mSocketHandler;
    delete mTcpSocketHandler;
    delete mStreamHandler;
    qDebug() << "Deleted everything";
    mImageHandler->removeAllPeers();
}

QVariantList SessionHandler::getAudioInputDevices()
{
    return mStreamHandler->getAudioInputDevices();
}

bool SessionHandler::joinSession(QString _roomId, QString _roomPassword)
{

    qDebug() << "RoomId: " << _roomId;


    QSqlQuery q(mDb->mDb);
    q.prepare("SELECT r.id, r.password, u.username FROM room AS r, user AS u WHERE r.host = u.id AND r.id = :roomId AND r.password = :roomPassword");
    q.bindValue(":roomId", _roomId);
    q.bindValue(":roomPassword", _roomPassword);
    if (q.exec() && q.size() > 0)
    {
        if (q.size() > 0)
        {
            q.next();
            mRoomId = q.value(0).toString();
            mRoomPassword = q.value(1).toString();
            mRoomHostUsername = q.value(2).toString();
            addUser();

            mSettings->setLastRoomId(mRoomId);
            mSettings->setLastRoomPassword(mRoomPassword);
            mSettings->saveSettings();
            qDebug() << "Adding peer with display name" << mSettings->getDisplayName();
            mImageHandler->addPeer(0, mSettings->getDisplayName());


            //Init everything that needs init
            initOtherStuff();




            return true;
        }
        else
        {
            qDebug() << "No such room combo";
        }
    }
    else
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO;
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
        QSqlQuery q(mDb->mDb);
        q.prepare("INSERT INTO roomSession (roomId, userId) VALUES (:roomId, :userId)");
        q.bindValue(":roomId", mRoomId);
        q.bindValue(":userId", mUser->getUserId());
        if (q.exec())
        {
            qDebug() << "Added user to the session";
        }
        else
        {
            qDebug() << "Failed Query" << Q_FUNC_INFO << " reason: " << q.lastError();
        }

    }
    else
    {
        if (addGuestUserToDatabase())
        {
            QSqlQuery q(mDb->mDb);
            q.prepare("INSERT INTO roomSession (roomId, userId) VALUES (:roomId, :userId)");
            q.bindValue(":roomId", mRoomId);
            q.bindValue(":userId", mUser->getGuestId());
            if (q.exec())
            {
                qDebug() << "Added guest to the session";
            }
            else
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


    if (mUser->isGuest())
    {
        QSqlQuery q(mDb->mDb);
        q.prepare("DELETE FROM roomSession WHERE roomId = :roomId AND userId = :userId");
        q.bindValue(":roomId", mRoomId);
        q.bindValue(":userId", mUser->getGuestId());
        if (q.exec())
        {
            qDebug() << "Removed guest from the session";
            setDefaultRoomID();
            qDebug() << "Setting the default roomId";
            return true;
        }
        else
        {
            qDebug() << "Failed Query" << Q_FUNC_INFO;
        }
    }
    else
    {
        QSqlQuery q(mDb->mDb);
        q.prepare("DELETE FROM roomSession WHERE roomId = :roomId AND userId = :userId");
        q.bindValue(":roomId", mRoomId);
        q.bindValue(":userId", mUser->getUserId());
        if (q.exec())
        {
            qDebug() << "Removed user from the session";
            return true;
        }
        else
        {
            qDebug() << "Failed Query" << Q_FUNC_INFO;
        }
    }
    return false;
}

bool SessionHandler::createSession(QString roomId, QString roomPassword)
{
    if (!mUser->isGuest())
    {
        if (!mUser->hasRoom())
        {
            QSqlQuery q(mDb->mDb);
            q.prepare("INSERT INTO room (id, host, password) VALUES (:id, :host, :password)");
            q.bindValue(":id", roomId);
            q.bindValue(":host", mUser->getUserId());
            q.bindValue(":password", roomPassword);
            if (q.exec())
            {
                qDebug() << "Added room :" << roomId << "to the database";
                return joinSession(roomId, roomPassword);
            }
            else
            {
                qDebug() << "Failed Query" << Q_FUNC_INFO;
            }
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
    QSqlQuery q(mDb->mDb);
    q.prepare("INSERT INTO user (streamId, username, password) VALUES (substring(MD5(RAND()),1,16), :username, substring(MD5(RAND()),1,16))");
    q.bindValue(":username", mUser->getGuestName());
    if (q.exec())
    {
        qDebug() << "Added guest :" << mUser->getGuestName() << "to the database";
        return true;
    }
    else
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO;
    }
    return false;
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
        mImageHandler->updatePeerDisplayName(0, mSettings->getDisplayName());
        mTcpSocketHandler->updateDisplayName(mSettings->getDisplayName());
        mTcpSocketHandler->sendChangedDisplayNameSignal();
    }
}

bool SessionHandler::checkVideoEnabled()
{
    return mStreamHandler->checkVideoEnabled();
}

bool SessionHandler::checkAudioEnabled()
{
    return mStreamHandler->checkAudioEnabled();
}
