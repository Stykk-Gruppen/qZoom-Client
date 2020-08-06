#ifndef SERVERTCPQUERIES_H
#define SERVERTCPQUERIES_H
#include <QTcpSocket>
#include <QHostAddress>

class ServerTcpQueries : public QTcpSocket
{
public:
    ServerTcpQueries(int port, QHostAddress address, QObject* parent=nullptr);
    /*QVariantList query0SelectFromRoom(QString _roomId, QString _roomPassword);
    int query1InsertIntoRoomSession(QString _roomId, QString _userId);
    int query2InsertIntoRoom(QString _roomId, QString _host, QString _roomPassword);
    int query3InsertIntoUser(QString _username);
    QVariantList query4SelectFromUser(QString _username);
    QVariantList query5SelectFromUser(QString _userId);
    QVariantList query6SelectFromRoom(QString _userId);
    int query7UpdateRoom(QString roomId, QString roomPassword, QString _host);
    QString query8SelectFromUser(QString _userId);
    int query9SelectFromUser(QString _username);*/
    int CUDQuery(int code, QVariantList vars);
    QVariantList RQuery(int code, QVariantList vars);

private:
    bool connect();
    bool disconnect();
    QVariantList parseData(QByteArray arr) const;
    int mPortNumber;
    int mMillisWait;
    QHostAddress mServerAddress;
};

#endif // SERVERTCPQUERIES_H
