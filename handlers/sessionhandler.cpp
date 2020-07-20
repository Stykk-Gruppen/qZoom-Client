#include "sessionhandler.h"

SessionHandler::SessionHandler(Database* _db, UserHandler* _user, QObject *parent) : QObject(parent)
{
    mDb = _db;
    mUser = _user;
    //Set default room id
    mRoomId = "Debug";

    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses())
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
        {
             mIpAddress = address.toString();
        }
    }
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
        qDebug() << "Failed Query" << "SessionHandler::joinSession";
    }
    return false;
}

QString SessionHandler::getRoomId()
{
    return mRoomId;
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
        q.prepare("INSERT INTO roomSession (roomId, userId, ipAddress) VALUES (:roomId, :userId, :ipAddress)");
        q.bindValue(":roomId", mRoomId);
        q.bindValue(":userId", mUser->getUserId());
        q.bindValue(":ipAddress", mIpAddress);
        if (q.exec())
        {
            qDebug() << "Added user to the session";
        }
        else
        {
            qDebug() << "Failed Query" << "SessionHandler::addUser";
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
            qDebug() << "Failed Query" << "SessionHandler::leaveSession";
        }
    }
    return false;
}
