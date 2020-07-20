#include "userhandler.h"

UserHandler::UserHandler(Database* _db, QObject *parent) : QObject(parent)
{
    mDb = _db;
    mIsGuest = true;
    mErrorMessage = "No error message was set";
}

UserHandler::~UserHandler()
{

}

bool UserHandler::isGuest()
{
    return mIsGuest;
}

bool UserHandler::login(QString username, QString password)
{
    QSqlQuery q(mDb->mDb);
    q.prepare("SELECT id, password FROM user WHERE username = :username");
    q.bindValue(":username", username);
    if (q.exec())
    {
        q.next();
        int userId = q.value(0).toInt();
        //We have to do some hashing here someday
        if (password == q.value(1).toString())
        {
            if (fillUser(userId))
            {
                mIsGuest = false;
                return true;
            }
            else
            {
                mErrorMessage = "An unknown error has occured";
            }
        }
        else
        {
            mErrorMessage = "Password did not match!";
        }
    }
    else
    {
        qDebug() << "Failed Query" << "UserHandler::login";
    }
    return false;
}

bool UserHandler::fillUser(int userId)
{
    QSqlQuery q(mDb->mDb);
    q.prepare("SELECT streamId, username, password, timeCreated FROM user WHERE id = :userId");
    q.bindValue(":userId", userId);
    if (q.exec())
    {
        mUserId = userId;
        q.next();
        mStreamId = q.value(0).toString();
        mUsername = q.value(1).toString();
        mPassword = q.value(2).toString();
        mTimeCreated = q.value(3).toString();
        return true;
    }
    else
    {
        qDebug() << "Failed Query" << "UserHandler::fillUser";
        return false;
    }
}

QString UserHandler::getErrorMessage()
{
    return mErrorMessage;
}

int UserHandler::getUserId()
{
    return mUserId;
}
