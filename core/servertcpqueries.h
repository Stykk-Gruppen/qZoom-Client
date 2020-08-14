#ifndef SERVERTCPQUERIES_H
#define SERVERTCPQUERIES_H
#include <QTcpSocket>
#include <QHostAddress>
#include "settings.h"

class ServerTcpQueries : public QTcpSocket
{
public:
    ServerTcpQueries(Settings* settings, QObject* parent = nullptr);
    QVariantList serverQuery(int code, const QVariantList& vars);

private:
    bool connect();
    bool disconnect();
    QVariantList parseData(QByteArray arr) const;
    int mPortNumber;
    int mMillisWait;
    QHostAddress mServerAddress;
    Settings* mSettings;
};

#endif // SERVERTCPQUERIES_H
