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

int ServerTcpQueries::CUDQuery(int code, const QVariantList& vars)
{
    int numberOfRowsAffected = -1;
    QByteArray data;
    for(int i = 0; i < vars.size(); i++)
    {
        data.prepend(vars[i].toString().toLocal8Bit().data());
        data.prepend(vars[i].toString().size());
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
            numberOfRowsAffected = response[0];
        }
    }
    else
    {
        return numberOfRowsAffected;
    }
    disconnect();
    return numberOfRowsAffected;
}

QVariantList ServerTcpQueries::RQuery(int code, const QVariantList& vars)
{
    QVariantList returnList;
    QByteArray data;
    for(int i = 0; i < vars.size(); i++)
    {
        data.prepend(vars[i].toString().toLocal8Bit().data());
        data.prepend(vars[i].toString().size());
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
    disconnect();
    return returnList;
}















