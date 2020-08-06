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

    QVariantList vars;
    vars.append(username);
    QVariantList queryData = mServerTcpQueries->RQuery(4, vars);
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
    int userId = queryData[0].toInt();
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
    QVariantList vars;
    vars.append(QString::number(userId));
    QVariantList queryData = mServerTcpQueries->RQuery(5, vars);
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
    QVariantList vars;
    vars.append(QString::number(mUserId));
    QVariantList queryData = mServerTcpQueries->RQuery(6, vars);
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
    QVariantList vars;
    vars.append(roomId);
    vars.append(roomPassword);
    vars.append(QString::number(mUserId));
    int numberOfRowsAffected = mServerTcpQueries->CUDQuery(7, vars);
    if(numberOfRowsAffected<=0)
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO;
        return false;
    }
    mPersonalRoomId = roomId;
    mPersonalRoomPassword = roomPassword;
    return true;
}

QString UserHandler::getErrorMessage()
{
    return mErrorMessage;
}

int UserHandler::getUserId()
{
    return mUserId;
}

QString UserHandler::getStreamId()
{
    return mStreamId;
}

QString UserHandler::getPersonalRoomId()
{
    return mPersonalRoomId;
    qDebug() << "personal room id" << mPersonalRoomId;
}

QString UserHandler::getPersonalRoomPassword()
{
    return mPersonalRoomPassword;
}

bool UserHandler::hasRoom()
{
    return mHasRoom;
}

QString UserHandler::getGuestName()
{
    return mGuestName;
}

QString UserHandler::getGuestStreamId()
{
    QVariantList vars;
    vars.append(QString::number(getGuestId()));
    QString queryData = mServerTcpQueries->RQuery(8, vars)[0].toString();
    if(!queryData.isEmpty())
    {
        return queryData;
    }
    qDebug() << "Failed Query " << Q_FUNC_INFO;
    return "getGuestStreamIdFailed";
}

int UserHandler::getGuestId()
{
    QVariantList vars;
    vars.append(mGuestName);
    return mServerTcpQueries->RQuery(9, vars)[0].toInt();
}

bool UserHandler::logout()
{
    mIsGuest = true;
    return true;
}
