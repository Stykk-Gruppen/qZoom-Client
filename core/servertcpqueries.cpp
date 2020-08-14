#include "servertcpqueries.h"

ServerTcpQueries::ServerTcpQueries(Settings* settings, QObject* parent): QTcpSocket(parent)
{
    mSettings = settings;
    mMillisWait = 2000;
}
/**
 * Connects to the server, waits mMillisWait
 * for the connection to be successful
 * @return true on success
 */
bool ServerTcpQueries::connect()
{
    mServerAddress = (mSettings->getServerIpAddress() == "Localhost") ?
                QHostAddress::LocalHost : QHostAddress(mSettings->getServerIpAddress());
    mPortNumber = mSettings->getSqlTcpPort();
    this->connectToHost(mServerAddress, mPortNumber);
    if(!this->waitForConnected(mMillisWait))
    {
        qDebug() << "SQLTcpSocketError: " << this->errorString() << Q_FUNC_INFO;
        qDebug() << mServerAddress << mPortNumber;
        return false;
    }
    return true;
}
/**
 * Disconnects from the server, waits mMillisWait
 * for the disconnect to be successful
 * @return true on success
 */
bool ServerTcpQueries::disconnect()
{
    this->disconnectFromHost();
    if (this->state() == QAbstractSocket::UnconnectedState || this->waitForDisconnected(mMillisWait)) {
        return true;
    }
    qDebug() << "SQLTcpSocketError: " << this->errorString() << Q_FUNC_INFO;
    return false;
}
/**
 * Parse the QByteArray and splits the data into QString
 * @param arr QByteArray response from the server
 * @return QVariantList containing all the QStrings
 */
QVariantList ServerTcpQueries::parseData(QByteArray arr) const
{
    QVariantList vec;
    int size = arr[0];
    arr.remove(0, 1);
    for (int i = 0; i < size; i++)
    {
        int length = arr[0];
        arr.remove(0, 1);
        QByteArray tmpArr = QByteArray(arr, length);
        QString str(tmpArr);
        arr.remove(0, length);
        vec.push_back(str);
    }
    return vec;
}

/**
 * Sends x amount of variables together with y code over TCP to the server.
 * To be used in a SQL Query, where the int will tell the server which query to run.
 * On the server the variables will be bound from left to right. So the first variable the
 * server recieves will be bound to the first variable in the query. Example
 * q.prepare("UPDATE room SET id = :roomId, password = :roomPassword WHERE host = :host");
        q.bindValue(":roomId", vec[0]);
        q.bindValue(":roomPassword", vec[1]);
        q.bindValue(":host", vec[2].toInt());
 * @param code int describing which query to run
 * @param vars QVariantList input vars to be used in the query.
 * @return QVariantList which contains all the return variables of a select or the
 * number of rows affected in case of update, delete or insert. On failure
 * it will return a list with -1.
 */
QVariantList ServerTcpQueries::serverQuery(int code, const QVariantList& vars)
{
    QVariantList returnList;
    QByteArray data;
    for(int i = 0; i < vars.size(); i++)
    {
        data.append(vars[i].toString().size());
        data.append(vars[i].toString().toLocal8Bit().data());
    }
    data.prepend(vars.size());
    data.prepend(code);
    if(connect())
    {
        qDebug() <<"SQL TCP Query(" << code << ")" << " Sending data: " << data;
        this->write(data);
        if(this->waitForReadyRead(mMillisWait))
        {
            QByteArray response = this->readAll();
            returnList = parseData(response);
        }
    }
    else
    {
        returnList.append(int(-1));
    }
    disconnect();
    return returnList;
}















