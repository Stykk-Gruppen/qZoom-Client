#include "sockethandler.h"

SocketHandler::SocketHandler(std::mutex* _videoWriteLock,std::mutex* _audioWriteLock,QObject *parent) : QObject(parent)
{
    mAudioWriteLock = _audioWriteLock;
    mVideoWriteLock = _videoWriteLock;
    address = QHostAddress::LocalHost;
    //158.36.165.235
    //address = QHostAddress("158.36.165.235"); Tarald
    //address = QHostAddress("79.160.58.120"); Kent
    port = 1337;
    initSocket();
}

void SocketHandler::initSocket()
{
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(address, port,QAbstractSocket::ShareAddress);

    //Connects readyRead to readPendingDatagram function,
    //which means when the socket recieves a packet the function will run.
    connect(udpSocket, &QUdpSocket::readyRead, this, &SocketHandler::readPendingDatagrams);

}

void SocketHandler::readPendingDatagrams()
{

    while (udpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();

        //Checks the first byte in the datagram to determine if the datagram is audio or video
        int audioOrVideoInt = datagram.data()[0];
        QByteArray arrayWithoutHeaderByte;
        arrayWithoutHeaderByte.append(datagram.data());
        arrayWithoutHeaderByte.remove(0,1);
        if(audioOrVideoInt == 0)
        {
            mAudioWriteLock->lock();
            mAudioBuffer.append(arrayWithoutHeaderByte);
            mAudioWriteLock->unlock();
            if(!mAudioPlaybackStarted && mAudioBuffer.size() >= mInitialBufferSize)
            {
                emit startAudioPlayback();
                mAudioPlaybackStarted = true;
            }
            qDebug() << "audio buffer size " << mAudioBuffer.size() << "after signal: " << signalCount;
        }
        else if (audioOrVideoInt ==1)
        {
            mVideoWriteLock->lock();
            mVideoBuffer.append(arrayWithoutHeaderByte);
            mVideoWriteLock->unlock();
            if(!mVideoPlaybackStarted && mVideoBuffer.size() >= mInitialBufferSize)
            {
                emit startVideoPlayback();
                mVideoPlaybackStarted = true;
            }
            qDebug() << "video buffer size " << mVideoBuffer.size() << "after signal: " << signalCount;
        }
        else
        {
            qDebug() << "UDP Header byte was not 1 or 0 in socketHandler, stopping program";
            exit(-1);
        }
    }

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
