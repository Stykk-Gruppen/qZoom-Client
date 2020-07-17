#include "sessionhandler.h"

SessionHandler::SessionHandler(Database* _db, QObject *parent) : QObject(parent)
{
    mDb = _db;
    //Set default room id
    mRoomId = "Failed to find Room ID :( ";
}

bool SessionHandler::joinSession(QString _roomId, QString _roomPassword)
{
    QSqlQuery q(mDb->mDb);
    q.prepare("SELECT id, password FROM room WHERE id = :roomId AND password = :roomPassword");
    q.bindValue(":roomId", _roomId);
    q.bindValue(":roomPassword", _roomPassword);
    if (q.exec() && q.size() > 0)
    {
        q.next();
        mRoomId = q.value(0).toString();
        return true;
    }
    else
    {
        return false;
        qDebug() << "No such combo";
    }
}

QString SessionHandler::getRoomId()
{
    return mRoomId;
}
