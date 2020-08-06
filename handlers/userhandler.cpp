#include "userhandler.h"

UserHandler::UserHandler(ServerTcpQueries* _mServerTcpQueries, Settings* settings, QObject *parent) : QObject(parent)
{
    mServerTcpQueries = _mServerTcpQueries;
    mSettings = settings;
    mIsGuest = true;
    mStreamId = "StreamID has not been set. This should never occur.";
    mErrorMessage = "No error message was set";
    mGuestName = "Guest" + QString::number(QDateTime::currentMSecsSinceEpoch());
}

UserHandler::~UserHandler()
{

}

bool UserHandler::isGuest()
{
    return mIsGuest;
}

bool UserHandler::login(QString username, QString password)
{
    const QVariantList queryData = mServerTcpQueries->querySelectFrom_user1(username);
    //Hvis lista er tom, så fant man ikke brukeren
    if(queryData.isEmpty())
    {
        errorHandler->giveErrorDialog("Username or password is wrong");
        return false;
    }
    else if(queryData[0].toInt() == -1)
    {
        errorHandler->giveErrorDialog("Could not connect to server");
        return false;
    }
    const int userId = queryData[0].toInt();
    //qDebug() << "login userId " << userId;
    //We have to do some hashing here someday
    if (password == queryData[1].toString())
    {
        if (fillUser(userId))
        {
            mIsGuest = false;
            mHasRoom = getPersonalRoom();
            qDebug() << "has room: " << mHasRoom;
            if(mSettings->getDisplayName().contains("guest", Qt::CaseInsensitive))
            {
                mSettings->setDisplayName(username);
                mSettings->saveSettings();
            }
            return true;
        }
        else
        {
            mErrorMessage = "An unknown error has occured";
        }
    }
    else
    {
        mErrorMessage = "Password did not match!";
    }
    return false;
}

bool UserHandler::fillUser(int userId)
{
    const QVariantList queryData = mServerTcpQueries->querySelectFrom_user2(QString::number(userId));
    if(queryData.size() > 0)
    {
        mUserId = userId;
        mStreamId = queryData[0].toString();
        mUsername = queryData[1].toString();
        mPassword = queryData[2].toString();
        mTimeCreated = queryData[3].toString();
        return true;
    }
    return false;
}

bool UserHandler::getPersonalRoom()
{
   // qDebug() << "mUserId " << mUserId;
    const QVariantList queryData = mServerTcpQueries->querySelectFrom_room2(QString::number(mUserId));
    if(queryData.size()>0)
    {
        mPersonalRoomId = queryData[0].toString();
        mPersonalRoomPassword = queryData[1].toString();
        return true;
    }
    return false;
}

bool UserHandler::updatePersonalRoom(QString roomId, QString roomPassword)
{
    if (roomId.length() == 0 || roomPassword.length() == 0)
    {
        return false;
    }
    const int numberOfRowsAffected = mServerTcpQueries->queryUpdate_room(roomId, roomPassword, QString::number(mUserId));
    if(numberOfRowsAffected <= 0)
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO;
        return false;
    }
    mPersonalRoomId = roomId;
    mPersonalRoomPassword = roomPassword;
    return true;
}

QString UserHandler::getErrorMessage() const
{
    return mErrorMessage;
}

int UserHandler::getUserId() const
{
    return mUserId;
}

QString UserHandler::getStreamId() const
{
    return mStreamId;
}

QString UserHandler::getPersonalRoomId() const
{
    return mPersonalRoomId;
    qDebug() << "personal room id" << mPersonalRoomId;
}

QString UserHandler::getPersonalRoomPassword() const
{
    return mPersonalRoomPassword;
}

bool UserHandler::hasRoom()
{
    return mHasRoom;
}

QString UserHandler::getGuestName() const
{
    return mGuestName;
}

QString UserHandler::getGuestStreamId() const
{
    const QString queryData = mServerTcpQueries->querySelectFrom_user3(QString::number(getGuestId()));
    if(!queryData.isEmpty())
    {
        return queryData;
    }
    qDebug() << "Failed Query " << Q_FUNC_INFO;
    return "getGuestStreamIdFailed";
}

int UserHandler::getGuestId() const
{
    return mServerTcpQueries->querySelectFrom_user4(mGuestName);
}

bool UserHandler::logout()
{
    mIsGuest = true;
    return true;
}
