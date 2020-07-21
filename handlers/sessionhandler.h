#ifndef SESSIONHANDLER_H
#define SESSIONHANDLER_H

#include <QObject>
#include "core/database.h"
#include "handlers/userhandler.h"
#include <QHostInfo>
#include <QHostAddress>
#include <QNetworkInterface>

class SessionHandler : public QObject
{
    Q_OBJECT
public:
    explicit SessionHandler(Database* _db, UserHandler* _user, QObject *parent = nullptr);
    Q_INVOKABLE bool joinSession(QString _roomId, QString _roomPassword);
    Q_INVOKABLE bool leaveSession();
    Q_INVOKABLE QString getRoomId();
    UserHandler* getUser();
private:
    QString mRoomId;
    QString mIpAddress;
    Database* mDb;
    UserHandler* mUser;
    void addUser();

signals:

};

#endif // SESSIONHANDLER_H
