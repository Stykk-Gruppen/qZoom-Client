#ifndef USERHANDLER_H
#define USERHANDLER_H

#include <QObject>
#include <QString>
#include "core/database.h"
#include "settings.h"
class UserHandler : public QObject
{
    Q_OBJECT
public:
    explicit UserHandler(Database* _db, Settings* settings, QObject *parent = nullptr);
    ~UserHandler();
    Q_INVOKABLE bool login(QString username, QString password);
    Q_INVOKABLE QString getErrorMessage();
    bool isGuest();
    bool hasRoom();
    int getUserId();
    QString getStreamId();
    QString getPersonalRoomId();
    QString getPersonalRoomPassword();

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
    Settings* mSettings;

signals:

};

#endif // USERHANDLER_H
