#ifndef DATABASE_H
#define DATABASE_H

#include <QDebug>
#include <QtSql>
#include <QSqlQuery>

class Database
{
public:
    Database();
    bool connectDatabase();
    QSqlDatabase mDb;
private:
};

#endif // DATABASE_H
