#include "database.h"
#include "config.cpp"

Database::Database()
{
    std::string str = connectDatabase() ? "yes" : "no";
    qDebug() << str.c_str();
}

bool Database::connectDatabase()
{
    mDb = QSqlDatabase::addDatabase("QMYSQL");
    mDb.setHostName(dbHostName);
    mDb.setDatabaseName(dbDatabaseName);
    mDb.setUserName(dbUserName);
    mDb.setPassword(dbPassword);
    qDebug() << mDb.lastError();
    return mDb.open();
}
