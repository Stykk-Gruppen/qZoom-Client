#include "sockethandler.h"

SocketHandler::SocketHandler(int bufferSize,ImageHandler* _imageHandler,
                             SessionHandler* _sessionHandler,QObject *parent) : QObject(parent)
{
    mBufferSize = bufferSize;
    mImageHandler = _imageHandler;
    mSessionHandler = _sessionHandler;
    address = QHostAddress::LocalHost;
    //address = QHostAddress("46.250.220.57"); //tarves.no
    //address = QHostAddress("158.36.165.235"); Tarald
    //address = QHostAddress("79.160.58.120"); Kent
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
        QByteArray data = datagram.data();

        if(1){
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
            mAudioMutexVector[index]->lock();
            mAudioBufferVector[index]->append(data);
            mAudioMutexVector[index]->unlock();
            if(!mAudioPlaybackStartedVector[index] && mAudioBufferVector[index]->size() >= mBufferSize)
            {
                QtConcurrent::run(mAudioPlaybackHandlerVector[index], &AudioPlaybackHandler::start);
                mAudioPlaybackStartedVector[index] = true;
            }
            // qDebug() << "audio buffer size " << mAudioBufferVector[index].size() << "after signal: " << signalCount;
        }
        else if (audioOrVideoInt ==1)
        {
            mVideoMutexVector[index]->lock();
            mVideoBufferVector[index]->append(data);
            mVideoMutexVector[index]->unlock();
            if(!mVideoPlaybackStartedVector[index] && mVideoBufferVector[index]->size() >= mBufferSize*4)
            {
                QtConcurrent::run(mVideoPlaybackHandlerVector[index], &VideoPlaybackHandler::start);
                mVideoPlaybackStartedVector[index] = true;
            }
            //qDebug() << "video buffer size " << mVideoBufferVector[index]->size() << "after signal: " << signalCount;
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
    if(mStreamIdVector.size()>=1)
    {
        for(size_t i=0;i<mStreamIdVector.size();i++)
        {
            if(QString::compare(streamId, mStreamIdVector[i], Qt::CaseSensitive)==0)
            {
                return i;
            }
        }
        //If the streamId does not exist, push it and buffers/locks
        addStreamToVector(streamId,mStreamIdVector.size());
        return mStreamIdVector.size();
    }
    else
    {
        //If this stream is the first to join, push it and buffer/locks
        addStreamToVector(streamId,0);
        return 0;
    }

}

void SocketHandler::addStreamToVector(QString streamId,int index)
{
    QByteArray* tempAudioBuffer = new QByteArray();
    QByteArray* tempVideoBuffer = new QByteArray();
    mAudioBufferVector.push_back(tempAudioBuffer);
    mVideoBufferVector.push_back(tempVideoBuffer);
    std::mutex* tempAudioLock = new std::mutex;
    std::mutex* tempVideoLock = new std::mutex;
    mAudioMutexVector.push_back(tempAudioLock);
    mVideoMutexVector.push_back(tempVideoLock);
    mAudioPlaybackHandlerVector.push_back(new AudioPlaybackHandler(tempAudioLock,tempAudioBuffer,mBufferSize));
    mVideoPlaybackHandlerVector.push_back(new VideoPlaybackHandler(tempVideoLock,mImageHandler,tempVideoBuffer,mBufferSize,index+1));
    mStreamIdVector.push_back(streamId);
    mAudioPlaybackStartedVector.push_back(false);
    mVideoPlaybackStartedVector.push_back(false);

    //Your own image is at 0, so we add 1 here and in videoPlayback constructor
    mImageHandler->addPeer(index+1);
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
            ret = udpSocket->writeDatagram(temp, temp.size(), address, port);
        }
        else
        {
            arr.prepend(arrToPrepend);
            ret = udpSocket->writeDatagram(arr, arr.size(), address, port);
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

