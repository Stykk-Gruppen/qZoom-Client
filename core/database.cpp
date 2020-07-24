#include "database.h"
#include "config.cpp"

Database::Database()
{
    std::string str = connectDatabase() ? "Connected to Database" : "Failed to connect to Database";
    qDebug() << str.c_str();
}

bool Database::connectDatabase()
{
    mDb = QSqlDatabase::addDatabase("QMYSQL");
    mDb.setHostName(dbHostName);
    mDb.setDatabaseName(dbDatabaseName);
    mDb.setUserName(dbUserName);
    mDb.setPassword(dbPassword);
    //qDebug() << mDb.lastError();
    return mDb.open();
}
