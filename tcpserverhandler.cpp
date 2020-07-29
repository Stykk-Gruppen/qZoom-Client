#include "tcpserverhandler.h"

TcpServerHandler::TcpServerHandler(InputStreamHandler* inputStreamHandler, int port, QObject *parent) : QObject(parent)
{
    mPort = port;
    mInputStreamHandler = inputStreamHandler;
}

void TcpServerHandler::init()
{
    mTcpServer = new QTcpServer();
    connect(mTcpServer, &QTcpServer::newConnection, this, &TcpServerHandler::acceptTcpConnection);
    mTcpServer->listen(QHostAddress::Any, mPort);
}

void TcpServerHandler::close()
{
    mTcpServer->close();
    delete mTcpServer;
}

void TcpServerHandler::acceptTcpConnection()
{
    mTcpServerConnection = mTcpServer->nextPendingConnection();
    if (!mTcpServerConnection) {
        qDebug() << "Error: got invalid pending connection!";
    }

    connect(mTcpServerConnection, &QIODevice::readyRead, this, &TcpServerHandler::readTcpPacket);
    //connect(tcpServerConnection, &QAbstractSocket::errorOccurred, this, &SocketHandler::displayError);
    connect(mTcpServerConnection, &QTcpSocket::disconnected, mTcpServerConnection, &QTcpSocket::deleteLater);
}

void TcpServerHandler::readTcpPacket()
{
    QByteArray data = mTcpServerConnection->readAll();
    qDebug() << mTcpServerConnection->peerAddress();
    QByteArray originalData = data;
    QByteArray header;



    int code = data[0];
    data.remove(0, 1);
    switch(code)
    {
    case 0:
    {
        //HEADER
        int numOfHeaders = data[0];
        qDebug() << "number of headers recieved from server: " << numOfHeaders;
        data.remove(0,1);
        //QString data(reply);


        //qDebug() << "DataString: " << data;
        //qDebug() << reply.indexOf(27);
        for(int i = 0; i < numOfHeaders; i++)
        {
            QByteArray temp = QByteArray(data, data.indexOf(27));
            //qDebug() << "Temp: " << temp;
            mInputStreamHandler->handleHeader(temp);
            data.remove(0, data.indexOf(27));
        }
        //mSocket->write("0");
        break;
    }

    case 1:
    {
        //REmove the user with this streamId

    }
    default:
        qDebug() << "Dont understand the code sent in beginning of tcp mesasge";
    }

    ;
}


QByteArray TcpServerHandler::getMessage()
{
    return message;
}
