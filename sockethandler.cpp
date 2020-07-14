#include "sockethandler.h"

SocketHandler::SocketHandler(std::mutex* _writeLock,QObject *parent) : QObject(parent)
{
    writeLock = _writeLock;
    address = QHostAddress::LocalHost;
    port = 1337;
    initSocket();
}

void SocketHandler::initSocket()
{
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(address, port,QAbstractSocket::ShareAddress);
    // udpSocket->connectToHost(address,port,QIODevice::ReadWrite);
    connect(udpSocket, &QUdpSocket::readyRead, this, &SocketHandler::readPendingDatagrams);


   /* QString program = "ffmpeg";
    QStringList arguments;
    arguments << "-i" << "test.mkv" << "-f" << "matroska" << "udp://localhost:1337";
    QProcess *myProcess = new QProcess(this);
    myProcess->start(program, arguments);*/


    //starter ffmpeg streaming for debugging


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
    //qDebug() << "signal recieved: " << signalCount;
    while (udpSocket->hasPendingDatagrams())
    {

        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        writeLock->lock();
        mBuffer.append(datagram.data());
        writeLock->unlock();
        if(!mPlaybackStarted && mBuffer.size()>= 4*1024)
        {
            emit startPlayback();
            mPlaybackStarted = true;
        }
        //processTheDatagram(datagram);
    }
    qDebug() << "buffer size " << mBuffer.size() << "after signal: " << signalCount;
    signalCount++;
}

int SocketHandler::sendDatagram(QByteArray arr)
{
    int ret = udpSocket->writeDatagram(arr, arr.size(), address, port);
    if(ret<0){
        qDebug() << udpSocket->error();
    }
    return ret;
}
