#include "userhandler.h"

UserHandler::UserHandler(Database* _db, QObject *parent) : QObject(parent)
{
    mDb = _db;
    mIsGuest = true;
    mErrorMessage = "No error message was set";
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
    QSqlQuery q(mDb->mDb);
    q.prepare("SELECT id, password FROM user WHERE username = :username");
    q.bindValue(":username", username);
    if (q.exec())
    {
        q.next();
        int userId = q.value(0).toInt();
        //We have to do some hashing here someday
        if (password == q.value(1).toString())
        {
            if (fillUser(userId))
            {
                mIsGuest = false;
                mHasRoom = getPersonalRoom();
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
    }
    else
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO;
    }
    return false;
}

bool UserHandler::fillUser(int userId)
{
    QSqlQuery q(mDb->mDb);
    q.prepare("SELECT streamId, username, password, timeCreated FROM user WHERE id = :userId");
    q.bindValue(":userId", userId);
    if (q.exec())
    {
        mUserId = userId;
        q.next();
        mStreamId = q.value(0).toString();
        mUsername = q.value(1).toString();
        mPassword = q.value(2).toString();
        mTimeCreated = q.value(3).toString();
        return true;
    }
    else
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO;
        return false;
    }
}

bool UserHandler::getPersonalRoom()
{
    QSqlQuery q(mDb->mDb);
    q.prepare("SELECT id, password FROM room WHERE host = :userId");
    q.bindValue(":userId", mUserId);
    if (q.exec())
    {
        if (q.size() > 0)
        {
            q.next();
            mPersonalRoomId = q.value(0).toString();
            mPersonalRoomPassword = q.value(1).toString();
            return true;
        }
        else
        {
            qDebug() << "User doesn't have a personal room";
        }
    }
    else
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO;
    }
    return false;
}

bool UserHandler::updatePersonalRoom(QString roomId, QString roomPassword)
{
    if (roomId.length() == 0 || roomPassword.length() == 0)
    {
        return false;
    }
    QSqlQuery q(mDb->mDb);
    q.prepare("UPDATE room SET id = :roomId, password = :roomPassword WHERE host = :host");
    q.bindValue(":roomId", roomId);
    q.bindValue(":roomPassword", roomPassword);
    q.bindValue(":host", mUserId);
    if (q.exec())
    {
        mPersonalRoomId = roomId;
        mPersonalRoomPassword = roomPassword;
        return true;
    }
    else
    {
        qDebug() << "Failed Query" << Q_FUNC_INFO;
    }
    return false;
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
