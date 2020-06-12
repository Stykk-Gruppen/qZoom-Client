#include "sockethandler.h"

SocketHandler::SocketHandler(QObject *parent) : QObject(parent)
{

}

void SocketHandler::initSocket()
{
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::LocalHost, 7755);

    connect(udpSocket, &QUdpSocket::readyRead,
            this, &SocketHandler::readPendingDatagrams);
}

void SocketHandler::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        processTheDatagram(datagram);
    }
}

void SocketHandler::processTheDatagram(QNetworkDatagram datagram)
{
    //do something
}
