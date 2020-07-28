#include "tcpserverhandler.h"

TcpServerHandler::TcpServerHandler(QObject *parent) : QObject(parent)
{
    mPort = 1338;
    init();
}

void TcpServerHandler::init()
{
    mTcpServer = new QTcpServer();
    connect(mTcpServer, &QTcpServer::newConnection, this, &TcpServerHandler::acceptTcpConnection);
    mTcpServer->listen(QHostAddress::Any, mPort);
}

void TcpServerHandler::acceptTcpConnection()
{
    tcpServerConnection = mTcpServer->nextPendingConnection();
        if (!tcpServerConnection) {
            qDebug() << "Error: got invalid pending connection!";
        }

    connect(tcpServerConnection, &QIODevice::readyRead, this, &TcpServerHandler::readTcpPacket);
    //connect(tcpServerConnection, &QAbstractSocket::errorOccurred, this, &SocketHandler::displayError);
    connect(tcpServerConnection, &QTcpSocket::disconnected, tcpServerConnection, &QTcpSocket::deleteLater);

    mTcpServer->close();
}

void TcpServerHandler::readTcpPacket()
{
    message = tcpServerConnection->readAll();
}

void TcpServerHandler::wait()
{
    tcpServerConnection->waitForReadyRead(5000);
}

QByteArray TcpServerHandler::getMessage()
{
    return message;
}
