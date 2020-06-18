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
    udpSocket->bind(address, port,QAbstractSocket::ShareAddress);
    // udpSocket->connectToHost(address,port,QIODevice::ReadWrite);
    connect(udpSocket, &QUdpSocket::readyRead,this, &SocketHandler::readPendingDatagrams);

    //starter ffmpeg streaming for debugging
    QString program = "ffmpeg";
    QStringList arguments;
    arguments << "-i" << "test.ismv" << "-f" << "ismv" << "udp://localhost:1337";
    QProcess *myProcess = new QProcess(this);
    myProcess->start(program, arguments);

}

void SocketHandler::readPendingDatagrams()
{

    /*forever {
        numRead  = socket.read(buffer, 50);

        // do whatever with array

        numReadTotal += numRead;
        if (numRead == 0 && !socket.waitForReadyRead())
            break;
    }*/
    qDebug() << "signal recieved";
    while (udpSocket->hasPendingDatagrams())
    {

        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        mBuffer.append(datagram.data());
        qDebug() << "datagram read";
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
