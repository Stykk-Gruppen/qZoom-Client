#include "servertcpqueries.h"

ServerTcpQueries::ServerTcpQueries(int _port, QHostAddress serverAddress, QObject* parent): QTcpSocket(parent)
{
    mServerAddress = serverAddress;
    mPortNumber = _port;
    mMillisWait = 2000;

}

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
bool ServerTcpQueries::disconnect()
{
    this->disconnectFromHost();
    if (this->state() == QAbstractSocket::UnconnectedState || this->waitForDisconnected(mMillisWait)) {
        return true;
    }
    qDebug() << "TcpSocketError: " << this->errorString() << Q_FUNC_INFO;
    return false;
}
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
//bool SessionHandler::joinSession(QString _roomId, QString _roomPassword)
//q.prepare("SELECT r.id, r.password, u.username FROM room AS r, user AS u WHERE r.host = u.id AND r.id = :roomId AND r.password = :roomPassword");
QVariantList ServerTcpQueries::querySelectFrom_room1(QString _roomId, QString _roomPassword)
{
    QVariantList returnList;
    int code = 0;
    QByteArray data;
    //Array should be [code,pw size,pw,room size,room]
    data.prepend(_roomPassword.toLocal8Bit().data());
    data.prepend(_roomPassword.size());
    data.prepend(_roomId.toLocal8Bit().data());
    data.prepend(_roomId.size());
    data.prepend(int(2));
    data.prepend(code);
    if(connect())
    {
        qDebug() << "tcp sql 0 writing data; " << data;
        this->write(data);
        if(this->waitForReadyRead(mMillisWait))
        {
            QByteArray response = this->readAll();
            qDebug() << "tcp sql 0 reading data; " << response;
            returnList = parseData(response);
        }
        else
        {
            returnList.append(-1);
        }

    }
    else
    {
        returnList.append(-1);
    }
    disconnect();
    //qDebug() << returnList;
    //returns [roomId,roomPassword,username]
    return returnList;
}
//void SessionHandler::addUser()
// q.prepare("INSERT INTO roomSession (roomId, userId) VALUES (:roomId, :userId)");
int ServerTcpQueries::queryInsertInto_roomSession(QString _roomId, QString _userId)
{
    int numberOfRowsAffected = -1;
    int code = 1;
    QByteArray data;
    //Array should be [code,user size,user,room size,room]
    data.prepend(_userId.toLocal8Bit().data());
    data.prepend(_userId.size());
    data.prepend(_roomId.toLocal8Bit().data());
    data.prepend(_roomId.size());
    data.prepend(int(2));
    data.prepend(code);
    if(connect())
    {
        qDebug() << "tcp sql 1 writing data; " << data;
        this->write(data);
        if(this->waitForReadyRead(mMillisWait))
        {
            QByteArray response = this->readAll();
            qDebug() << "tcp sql 1 reading data; " << response;
            numberOfRowsAffected = response[0];
        }
    }
    disconnect();
    return numberOfRowsAffected;
}
//bool SessionHandler::createSession(QString roomId, QString roomPassword)
//q.prepare("INSERT INTO room (id, host, password) VALUES (:id, :host, :password)");
int ServerTcpQueries::queryInsertInto_room(QString _roomId, QString _host, QString _roomPassword)
{
    int numberOfRowsAffected = -1;
    int code = 2;
    QByteArray data;
    data.prepend(_roomPassword.toLocal8Bit().data());
    data.prepend(_roomPassword.size());
    data.prepend(_host.toLocal8Bit().data());
    data.prepend(_host.size());
    data.prepend(_roomId.toLocal8Bit().data());
    data.prepend(_roomId.size());
    data.prepend(int(3));
    data.prepend(code);
    if(connect())
    {
        qDebug() << "tcp sql 2 writing data; " << data;
        this->write(data);
        if(this->waitForReadyRead(mMillisWait))
        {
            QByteArray response = this->readAll();
            qDebug() << "tcp sql 2 reading data; " << response;
            numberOfRowsAffected = response[0];
        }
    }
    disconnect();
    return numberOfRowsAffected;
}
//bool SessionHandler::addGuestUserToDatabase()
//q.prepare("INSERT INTO user (streamId, username, password, isGuest) VALUES (substring(MD5(RAND()),1,16), :username, substring(MD5(RAND()),1,16), TRUE)");
int ServerTcpQueries::queryInsertInto_user(QString _username)
{
    int numberOfRowsAffected = -1;
    int code = 3;
    QByteArray data;
    data.prepend(_username.toLocal8Bit().data());
    data.prepend(_username.size());
    data.prepend(int(1));
    data.prepend(code);
    if(connect())
    {
        qDebug() << "tcp sql 3 writing data; " << data;
        this->write(data);
        if(this->waitForReadyRead(mMillisWait))
        {
            QByteArray response = this->readAll();
            qDebug() << "tcp sql 3 reading data; " << response;
            numberOfRowsAffected = response[0];
        }
    }
    disconnect();
    return numberOfRowsAffected;
}
//bool UserHandler::login(QString username, QString password)
// q.prepare("SELECT id, password FROM user WHERE username = :username");
QVariantList ServerTcpQueries::querySelectFrom_user1(QString _username)
{
    QVariantList returnList;
    int code = 4;
    QByteArray data;
    data.prepend(_username.toLocal8Bit().data());
    data.prepend(_username.size());
    data.prepend(int(1));
    data.prepend(code);
    if(connect())
    {
        qDebug() << "tcp sql 4 writing data; " << data;
        this->write(data);
        if(this->waitForReadyRead(mMillisWait))
        {
            QByteArray response = this->readAll();
            qDebug() << "tcp sql 4 reading data; " << response;
            returnList = parseData(response);
        }
        else
        {
            returnList.prepend(-1);
        }

    }
    //Vet ikke om vi vil havne her noen gang?
    else
    {

        returnList.prepend(-1);
    }
    disconnect();
    //returns [id,password]
    qDebug() << returnList;
    return returnList;
}
//bool UserHandler::fillUser(int userId)
//q.prepare("SELECT streamId, username, password, timeCreated FROM user WHERE id = :userId");
QVariantList ServerTcpQueries::querySelectFrom_user2(QString _userId)
{
    QVariantList returnList;
    int code = 5;
    QByteArray data;
    data.prepend(_userId.toLocal8Bit().data());
    data.prepend(_userId.size());
    data.prepend(int(1));
    data.prepend(code);
    if(connect())
    {
        qDebug() << "tcp sql 5 writing data; " << data;
        this->write(data);
        if(this->waitForReadyRead(mMillisWait))
        {
            QByteArray response = this->readAll();
            qDebug() << "tcp sql 5 reading data; " << response;
            returnList = parseData(response);
        }

    }
    disconnect();
    return returnList;
}
//bool UserHandler::getPersonalRoom()
//q.prepare("SELECT id, password FROM room WHERE host = :userId");
QVariantList ServerTcpQueries::querySelectFrom_room2(QString _userId)
{
    QVariantList returnList;
    int code = 6;
    QByteArray data;
    data.prepend(_userId.toLocal8Bit().data());
    data.prepend(_userId.size());
    data.prepend(int(1));
    data.prepend(code);
    if(connect())
    {
        qDebug() << "tcp sql 6 writing data; " << data;
        this->write(data);
        if(this->waitForReadyRead(mMillisWait))
        {
            QByteArray response = this->readAll();
            if(response.size()>1)
            {
                qDebug() << "tcp sql 6 reading data; " << response;
                returnList = parseData(response);
            }
        }
    }
    disconnect();
    return returnList;
}

//bool UserHandler::updatePersonalRoom(QString roomId, QString roomPassword)
//q.prepare("UPDATE room SET id = :roomId, password = :roomPassword WHERE host = :host");
int ServerTcpQueries::queryUpdate_room(QString roomId, QString roomPassword, QString _host)
{
    int numberOfRowsAffected = -1;
    int code = 7;
    QByteArray data;
    data.prepend(_host.toLocal8Bit().data());
    data.prepend(_host.size());
    data.prepend(roomPassword.toLocal8Bit().data());
    data.prepend(roomPassword.size());
    data.prepend(roomId.toLocal8Bit().data());
    data.prepend(roomId.size());
    data.prepend(int(3));
    data.prepend(code);
    if(connect())
    {
        qDebug() << "tcp sql 7 writing data; " << data;
        this->write(data);
        if(this->waitForReadyRead(mMillisWait))
        {
            QByteArray response = this->readAll();
            qDebug() << "tcp sql 7 reading data; " << response;
            numberOfRowsAffected = response[0];
        }
    }
    disconnect();
    return numberOfRowsAffected;
}


//QString UserHandler::getGuestStreamId()
//q.prepare("SELECT streamId FROM user WHERE id = :id");
QString ServerTcpQueries::querySelectFrom_user3(QString _userId)
{
    QVariantList returnList;
    int code = 8;
    QByteArray data;
    data.prepend(_userId.toLocal8Bit().data());
    data.prepend(_userId.size());
    data.prepend(int(1));
    data.prepend(code);
    if(connect())
    {
        qDebug() << "tcp sql 8 writing data; " << data;
        this->write(data);
        if(this->waitForReadyRead(mMillisWait))
        {
            QByteArray response = this->readAll();
            qDebug() << "tcp sql 8 reading data; " << response;
            returnList = parseData(response);
        }

    }
    disconnect();
    return returnList[0].toString();
}

//int UserHandler::getGuestId()
//q.prepare("SELECT id FROM user WHERE username = :username");
int ServerTcpQueries::querySelectFrom_user4(QString _username)
{
    QVariantList returnList;
    int code = 9;
    QByteArray data;
    data.prepend(_username.toLocal8Bit().data());
    data.prepend(_username.size());
    data.prepend(int(1));
    data.prepend(code);
    if(connect())
    {
        qDebug() << "tcp sql 9 writing data; " << data;
        this->write(data);
        if(this->waitForReadyRead(mMillisWait))
        {
            QByteArray response = this->readAll();
            qDebug() << "tcp sql 9 reading data; " << response;
            returnList = parseData(response);
        }

    }
    disconnect();
    return returnList[0].toInt();
}

























