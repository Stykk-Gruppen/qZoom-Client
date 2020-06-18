#include "sockethandler.h"

SocketHandler::SocketHandler(QObject *parent) : QObject(parent)
{
    address = QHostAddress::LocalHost;
    port = 1337;
    initSocket();
}

void SocketHandler::initSocket()
{

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(address, port);
    //connect(udpSocket, &QUdpSocket::readyRead,this, &SocketHandler::readPendingDatagrams);
}

void SocketHandler::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        //processTheDatagram(datagram);
    }
}

int SocketHandler::sendDatagram(QByteArray arr)
{
    int ret = udpSocket->writeDatagram(arr, arr.size(), address, port);
    if(ret<0){
        qDebug() << udpSocket->error();
    }
    return ret;
}
