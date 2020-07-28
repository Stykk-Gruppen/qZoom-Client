#include "sessionhandler.h"

SessionHandler::SessionHandler(Database* _db, UserHandler* _user, QObject *parent) : QObject(parent)
{
    mDb = _db;
    mUser = _user;
    //Set default room id
    //mRoomId = "Debug";
    setDefaultRoomID();
    mIpAddress = "Ipaddress";

}

UserHandler* SessionHandler::getUser()
{
    return mUser;
}

bool SessionHandler::joinSession(QString _roomId, QString _roomPassword)
{
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
            q.prepare("INSERT INTO roomSession (roomId, userId, ipAddress) VALUES (:roomId, :userId, :ipAddress)");
            q.bindValue(":roomId", mRoomId);
            q.bindValue(":userId", mUser->getGuestId());
            q.bindValue(":ipAddress", mIpAddress);
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
