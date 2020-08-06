#include "servertcpqueries.h"

ServerTcpQueries::ServerTcpQueries(int _port, QHostAddress serverAddress, QObject* parent): QTcpSocket(parent)
{
    mServerAddress = serverAddress;
    mPortNumber = _port;
    mMillisWait = 2000;

}
/**
 * Connects to the server, waits mMillisWait
 * for the connection to be successful
 * @return true on success
 */
bool ServerTcpQueries::connect()
{
    this->connectToHost(mServerAddress, mPortNumber);
    if(!this->waitForConnected(mMillisWait))
    {
        qDebug() << "TcpSocketError: " << this->errorString() << Q_FUNC_INFO;
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
    qDebug() << "TcpSocketError: " << this->errorString() << Q_FUNC_INFO;
    return false;
}
/**
 * Parse the QByteArray and splits the data into QString
 * @param arr QByteArray response from the server
 * @return QVariantList containing all the QStrings
 */
QVariantList ServerTcpQueries::parseData(QByteArray arr)
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

int ServerTcpQueries::CUDQuery(int code, QVariantList vars)
{
    int numberOfRowsAffected = -1;
    QByteArray data;
    for(int i=0;i<vars.size();i++)
    {
        data.prepend(vars[i].toString().toLocal8Bit().data());
        data.prepend(vars[i].toString().size());
    }
    data.prepend(vars.size());
    data.prepend(code);
    if(connect())
    {
        this->write(data);
        if(this->waitForReadyRead(mMillisWait))
        {
            QByteArray response = this->readAll();
            numberOfRowsAffected = response[0];
        }

    }
    disconnect();
    return numberOfRowsAffected;
}
QVariantList ServerTcpQueries::RQuery(int code, QVariantList vars)
{
    QVariantList returnList;
    QByteArray data;
    for(int i=0;i<vars.size();i++)
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















