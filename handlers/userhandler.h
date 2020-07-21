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
    bool isGuest();
    int getUserId();
    QString getStreamId();
private:
    bool mIsGuest;
    int mUserId;
    Database* mDb;
    QString mErrorMessage;
    QString mStreamId;
    QString mUsername;
    QString mPassword;
    QString mTimeCreated;

    bool fillUser(int userId);

signals:

};

#endif // USERHANDLER_H
