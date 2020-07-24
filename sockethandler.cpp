#include "sockethandler.h"

SocketHandler::SocketHandler(int bufferSize,ImageHandler* _imageHandler, InputStreamHandler* inputStreamHandler,
                             SessionHandler* _sessionHandler, QHostAddress address,QObject *parent) : QObject(parent)
{
    mBufferSize = bufferSize;
    //mImageHandler = _imageHandler;
    mSessionHandler = _sessionHandler;
    mInputStreamHandler = inputStreamHandler;
    //address = QHostAddress::LocalHost;
    mAddress = address;
    port = 1337;
    initSocket();
}

void SocketHandler::initSocket()
{
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::Any, port,QAbstractSocket::ShareAddress);
    //Connects readyRead to readPendingDatagram function,
    //which means when the socket recieves a packet the function will run.
    connect(udpSocket, &QUdpSocket::readyRead, this, &SocketHandler::readPendingDatagrams);

}

void SocketHandler::readPendingDatagrams()
{

    while (udpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        if(datagram.senderAddress().toIPv4Address() != mAddress.toIPv4Address()) continue;
        QByteArray data = datagram.data();

        if(mAddress.toIPv4Address() != QHostAddress("46.250.220.57").toIPv4Address()){
            //roomId is the first x bytes, then streamId
            int roomIdLength = data[0];
            data.remove(0,1);

            //Finds the roomId header, stores it and removes it from the datagram
            QByteArray roomIdArray = QByteArray(data, roomIdLength);
            QString roomId(roomIdArray);
            data.remove(0, roomIdLength);
        }

        int streamIdLength = data[0];
        data.remove(0,1);

        //Finds the streamId header, stores it and removes it;
        QByteArray streamIdArray = QByteArray(data, streamIdLength);
        QString streamId(streamIdArray);
        data.remove(0, streamIdLength);

        int index = findStreamIdIndex(streamId);

        //Checks the first byte in the datagram to determine if the datagram is audio or video
        int audioOrVideoInt = data[0];
        data.remove(0,1);

        if(audioOrVideoInt == 0)
        {
            mInputStreamHandler->mAudioMutexVector[index]->lock();
            mInputStreamHandler->mAudioBufferVector[index]->append(data);
            mInputStreamHandler->mAudioMutexVector[index]->unlock();
            if(!mInputStreamHandler->mAudioPlaybackStartedVector[index] && mInputStreamHandler->mAudioBufferVector[index]->size() >= mBufferSize)
            {
                QtConcurrent::run(mInputStreamHandler->mAudioPlaybackHandlerVector[index], &AudioPlaybackHandler::start);
                mInputStreamHandler->mAudioPlaybackStartedVector[index] = true;
            }
            // qDebug() << "audio buffer size " << mAudioBufferVector[index].size() << "after signal: " << signalCount;
        }
        else if (audioOrVideoInt ==1)
        {
            mInputStreamHandler->mVideoMutexVector[index]->lock();
            mInputStreamHandler->mVideoBufferVector[index]->append(data);
            mInputStreamHandler->mVideoMutexVector[index]->unlock();
            if(!mInputStreamHandler->mVideoPlaybackStartedVector[index] && mInputStreamHandler->mVideoBufferVector[index]->size() >= mBufferSize)
            {

                QtConcurrent::run(mInputStreamHandler->mVideoPlaybackHandlerVector[index], &VideoPlaybackHandler::start);
                mInputStreamHandler->mVideoPlaybackStartedVector[index] = true;
            }
            qDebug() << "video buffer size " << mInputStreamHandler->mVideoBufferVector[index]->size() << "after signal: " << signalCount;
        }
        else
        {
            qDebug() << "UDP Header byte was not 1 or 0 in socketHandler, stopping program";
            exit(-1);
        }
    }

    signalCount++;
}

//Sockethandler has to put the datagram in the correct buffer when in a room with multiple people, based on the streamId
int SocketHandler::findStreamIdIndex(QString streamId)
{
    if(mInputStreamHandler->mStreamIdVector.size()>=1)
    {
        for(size_t i=0;i<mInputStreamHandler->mStreamIdVector.size();i++)
        {
            if(QString::compare(streamId, mInputStreamHandler->mStreamIdVector[i], Qt::CaseSensitive)==0)
            {
                return i;
            }
        }
    }
    qDebug() << "Vi er fucked2";
    exit(1);
}

int SocketHandler::sendDatagram(QByteArray arr)
{
    int ret;
    int audioOrVideIndex = -1;
    audioOrVideIndex = arr[0];
    arr.remove(0,1);
    QString streamId = mSessionHandler->getUser()->getStreamId();
    QString roomId = mSessionHandler->getRoomId();
    QByteArray arrToPrepend = QByteArray(audioOrVideIndex,1);

    //Puts the streamId and its size at the front of the array,
    //so the server knows where to send the stream
    arrToPrepend.prepend(streamId.toLocal8Bit().data());
    arrToPrepend.prepend(streamId.size());

    //Puts the roomId and its size at the front of the array
    arrToPrepend.prepend(roomId.toLocal8Bit().data());
    arrToPrepend.prepend(roomId.size());

    while(arr.size()>0){
        if(arr.size()>512-arrToPrepend.size())
        {
            QByteArray temp = QByteArray(arr,512-arrToPrepend.size());
            temp.prepend(arrToPrepend);
            arr.remove(0,512-arrToPrepend.size());
            ret = udpSocket->writeDatagram(temp, temp.size(), mAddress, port);
        }
        else
        {
            arr.prepend(arrToPrepend);
            ret = udpSocket->writeDatagram(arr, arr.size(), mAddress, port);
            break;
        }
        //qDebug() << ret << " size: " << arr.size();
        if(ret<0){
            qDebug() << udpSocket->error();
            break;
        }
    }
    return ret;
}

