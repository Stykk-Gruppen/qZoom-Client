#ifndef SESSIONHANDLER_H
#define SESSIONHANDLER_H

#include <QObject>
#include "core/database.h"

class SessionHandler : public QObject
{
    Q_OBJECT
public:
    explicit SessionHandler(Database* _db, QObject *parent = nullptr);
    Database* mDb;
    Q_INVOKABLE bool joinSession(QString _roomId, QString _roomPassword);
    Q_INVOKABLE QString getRoomId();

private:
    QString mRoomId;

signals:

};

#endif // SESSIONHANDLER_H
