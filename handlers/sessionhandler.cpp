#include "sessionhandler.h"

SessionHandler::SessionHandler(Database* _db, UserHandler* _user, QObject *parent) : QObject(parent)
{
    mDb = _db;
    mUser = _user;
    //Set default room id
    mRoomId = "Debug";

}

UserHandler* SessionHandler::getUser()
{
    return mUser;
}

bool SessionHandler::joinSession(QString _roomId, QString _roomPassword)
{
    QSqlQuery q(mDb->mDb);
    q.prepare("SELECT id, password FROM room WHERE id = :roomId AND password = :roomPassword");
    q.bindValue(":roomId", _roomId);
    q.bindValue(":roomPassword", _roomPassword);
    if (q.exec() && q.size() > 0)
    {
        if (q.size() > 0)
        {
            q.next();
            mRoomId = q.value(0).toString();
            mRoomPassword = q.value(1).toString();
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
    if (mUser->isGuest())
    {
        qDebug() << QHostInfo::localHostName() << QHostInfo::localDomainName();
    }
    else
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
}

bool SessionHandler::leaveSession()
{
    if (mUser->isGuest())
    {

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

