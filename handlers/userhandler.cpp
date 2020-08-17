#include "userhandler.h"

/**
 * @brief UserHandler::UserHandler
 * @param _mServerTcpQueries
 * @param settings
 * @param parent
 */
UserHandler::UserHandler(ServerTcpQueries* _mServerTcpQueries, Settings* settings, QObject *parent) : QObject(parent)
{
    mServerTcpQueries = _mServerTcpQueries;
    mSettings = settings;
    mIsGuest = true;
    mStreamId = "StreamID has not been set. This should never occur.";
    mErrorMessage = "No error message was set";
    mGuestName = "Guest" + QString::number(QDateTime::currentMSecsSinceEpoch());
}

/**
 * @brief UserHandler::~UserHandler
 */
UserHandler::~UserHandler()
{

}

/**
 * @brief UserHandler::isGuest
 * @return
 */
bool UserHandler::isGuest() const
{
    return mIsGuest;
}

/**
 * @brief UserHandler::login
 * @param username
 * @param password
 * @return
 */
bool UserHandler::login(const QString& username, const QString& password)
{
    QVariantList vars;
    vars.append(username);
    const QVariantList queryData = mServerTcpQueries->serverQuery(4, vars);
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

/**
 * @brief UserHandler::fillUser
 * @param userId
 * @return
 */
bool UserHandler::fillUser(int userId)
{
    QVariantList vars;
    vars.append(QString::number(userId));
    const QVariantList queryData = mServerTcpQueries->serverQuery(5, vars);
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

/**
 * @brief UserHandler::getPersonalRoom
 * @return
 */
bool UserHandler::getPersonalRoom()
{
    QVariantList vars;
    vars.append(QString::number(mUserId));
    const QVariantList queryData = mServerTcpQueries->serverQuery(6, vars);
    if(queryData.size() > 0)
    {
        mPersonalRoomId = queryData[0].toString();
        mPersonalRoomPassword = queryData[1].toString();
        return true;
    }
    return false;
}
/**
 * @brief UserHandler::updatePersonalRoom
 * @param roomId
 * @param roomPassword
 * @return
 */
bool UserHandler::updatePersonalRoom(const QString& roomId, const QString& roomPassword)
{
    if (roomId.length() == 0 || roomPassword.length() == 0)
    {
        return false;
    }
    QVariantList vars;
    vars.append(roomId);
    vars.append(roomPassword);
    vars.append(QString::number(mUserId));
    //The old roomID is used to check if the room is currently active.
    vars.append(mPersonalRoomId);
    const int numberOfRowsAffected = mServerTcpQueries->serverQuery(7, vars)[0].toInt();
    if(numberOfRowsAffected <= 0)
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO << " vars: " << vars;
        return false;
    }
    mPersonalRoomId = roomId;
    mPersonalRoomPassword = roomPassword;
    return true;
}

/**
 * @brief UserHandler::getErrorMessage
 * @return
 */
QString UserHandler::getErrorMessage() const
{
    return mErrorMessage;
}

/**
 * @brief UserHandler::getUserId
 * @return
 */
int UserHandler::getUserId() const
{
    return mUserId;
}

/**
 * @brief UserHandler::getStreamId
 * @return
 */
QString UserHandler::getStreamId() const
{
    return mStreamId;
}

/**
 * @brief UserHandler::getUsername
 * @return
 */
QString UserHandler::getUsername() const
{
    return mUsername;
}

/**
 * @brief UserHandler::getPersonalRoomId
 * @return
 */
QString UserHandler::getPersonalRoomId() const
{
    return mPersonalRoomId;
}

/**
 * @brief UserHandler::getPersonalRoomPassword
 * @return
 */
QString UserHandler::getPersonalRoomPassword() const
{
    return mPersonalRoomPassword;
}

/**
 * @brief UserHandler::hasRoom
 * @return
 */
bool UserHandler::hasRoom()
{
    return mHasRoom;
}

/**
 * @brief UserHandler::getGuestName
 * @return
 */
QString UserHandler::getGuestName() const
{
    return mGuestName;
}

/**
 * @brief UserHandler::getGuestStreamId
 * Gets ID from Database
 * @return
 */
QString UserHandler::getGuestStreamId() const
{
    QVariantList vars;
    vars.append(QString::number(getGuestId()));
    const QVariantList returnList = mServerTcpQueries->serverQuery(8, vars);
    if (returnList.isEmpty())
    {
        qDebug() << "Failed Query " << Q_FUNC_INFO;
        return "getGuestStreamIdFailed";
    }

    const QString queryData = returnList[0].toString();
    if (!queryData.isEmpty())
    {
        return queryData;
    }
    qDebug() << "Failed Query " << Q_FUNC_INFO;
    return "getGuestStreamIdFailed";
}

/**
 * @brief UserHandler::getDisplayName
 * @return
 */
QString UserHandler::getDisplayName() const
{
    return isGuest() ? mGuestName : mUsername;
}

/**
 * @brief UserHandler::getGuestId
 * Gets ID from Database
 * @return
 */
int UserHandler::getGuestId() const
{
    QVariantList vars;
    vars.append(mGuestName);
    const QVariantList returnList = mServerTcpQueries->serverQuery(9, vars);
    if (returnList.isEmpty())
    {
        qDebug() << "GuestID is empty";
        return -1;
    }
    return returnList[0].toInt();
}

/**
 * @brief UserHandler::logout
 * @return
 */
bool UserHandler::logout()
{
    mIsGuest = true;
    return true;
}
