#ifndef USERHANDLER_H
#define USERHANDLER_H

#include <QObject>
#include <QString>
#include "core/settings.h"
#include "core/servertcpqueries.h"
class UserHandler : public QObject
{
    Q_OBJECT
public:
    explicit UserHandler(ServerTcpQueries* _mServerTcpQueries, Settings* settings, QObject *parent = nullptr);
    ~UserHandler();
    Q_INVOKABLE bool login(const QString& username, const QString& password);
    Q_INVOKABLE bool logout();
    Q_INVOKABLE bool hasRoom();
    Q_INVOKABLE bool updatePersonalRoom(const QString& roomId, const QString& roomPassword);
    Q_INVOKABLE bool isGuest() const;
    Q_INVOKABLE QString getErrorMessage() const;
    Q_INVOKABLE QString getPersonalRoomId() const;
    Q_INVOKABLE QString getPersonalRoomPassword() const;
    int getUserId() const;
    int getGuestId() const;
    Q_INVOKABLE QString getStreamId() const;
    QString getUsername() const;
    QString getGuestName() const;
    Q_INVOKABLE QString getGuestStreamId() const;

private:
    ServerTcpQueries* mServerTcpQueries;
    int mUserId;
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
    QString mGuestName;
    QString mGuestId;
    Settings* mSettings;

signals:

};

#endif // USERHANDLER_H
