#ifndef SERVERTCPQUERIES_H
#define SERVERTCPQUERIES_H
#include <QTcpSocket>
#include <QHostAddress>

class ServerTcpQueries : public QTcpSocket
{
public:
    ServerTcpQueries(int port, QHostAddress address, QObject* parent=nullptr);
    QVariantList querySelectFrom_room1(QString _roomId, QString _roomPassword);
    int queryInsertInto_roomSession(QString _roomId, QString _userId);
    int queryInsertInto_room(QString _roomId, QString _host, QString _roomPassword);
    int queryInsertInto_user(QString _username);
    QVariantList querySelectFrom_user1(QString _username);
    QVariantList querySelectFrom_user2(QString _userId);
    QVariantList querySelectFrom_room2(QString _userId);
    int queryUpdate_room(QString roomId, QString roomPassword, QString _host);
    QString querySelectFrom_user3(QString _userId);
    int querySelectFrom_user4(QString _username);
private:
    int mPortNumber;
    QVariantList parseData(QByteArray arr);
    QHostAddress mServerAddress;
    bool connect();
    bool disconnect();
    int mMillisWait;
};

#endif // SERVERTCPQUERIES_H
