#ifndef USERHANDLER_H
#define USERHANDLER_H

#include <QObject>
#include <QString>
#include "core/database.h"

class UserHandler : public QObject
{
    Q_OBJECT
public:
    explicit UserHandler(Database* _db, QObject *parent = nullptr);
    ~UserHandler();
    Q_INVOKABLE bool login(QString username, QString password);
    Q_INVOKABLE QString getErrorMessage();
    Q_INVOKABLE QString getPersonalRoomId();
    Q_INVOKABLE QString getPersonalRoomPassword();
    Q_INVOKABLE bool hasRoom();
    Q_INVOKABLE bool updatePersonalRoom(QString roomId, QString roomPassword);
    bool isGuest();
    int getUserId();
    QString getStreamId();

private:
    bool fillUser(int userId);
    bool getPersonalRoom();

    bool mIsGuest;
    bool mHasRoom;
    QString mPersonalRoomId;
    QString mPersonalRoomPassword;
    QString mErrorMessage;
    QString mStreamId;
    QString mUsername;
    QString mPassword;
    QString mTimeCreated;
    int mUserId;
    Database* mDb;

signals:

};

#endif // USERHANDLER_H
