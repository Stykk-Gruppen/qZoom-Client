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
    Q_INVOKABLE bool createSession(QString _roomId, QString _roomPassword);
    Q_INVOKABLE bool leaveSession();
    Q_INVOKABLE bool isGuest();
    Q_INVOKABLE QString getRoomId();
    Q_INVOKABLE QString getRoomPassword();
    Q_INVOKABLE QString getRoomHostUsername();
    UserHandler* getUser();

private:
    void addUser();

    QString mRoomId;
    QString mRoomPassword;
    QString mRoomHostUsername;
    QString mIpAddress;

    Database* mDb;
    UserHandler* mUser;
    bool mUserHasRoom;
    void getUserRoom();
    bool addGuestUserToDatabase();


signals:

};

#endif // SESSIONHANDLER_H
