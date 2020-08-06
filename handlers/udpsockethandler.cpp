#include "udpsockethandler.h"

UdpSocketHandler::UdpSocketHandler(int bufferSize, int _port, InputStreamHandler* inputStreamHandler,
                             QString streamId, QString roomId, QHostAddress address,QObject *parent) : QObject(parent)
{
    mBufferSize = bufferSize;
    mInputStreamHandler = inputStreamHandler;
    mAddress = address;
    mPort = _port;
    mStreamId = streamId;
    mRoomId = roomId;
    initSocket();
}
/**
 * Creates a new QUdepSocket and makes it listen to port and QHostAddress::Any.
 * When we used mAddress(ip address to the server) it did not recieve any datagrams.
 * It also connects the readyRead signal to the readPendingDatagram function.
 */
void UdpSocketHandler::initSocket()
{
    mUdpSocket = new QUdpSocket(this);
    mUdpSocket->bind(QHostAddress::Any, mPort, QAbstractSocket::ShareAddress);
    connect(mUdpSocket, &QUdpSocket::readyRead, this, &UdpSocketHandler::readPendingDatagrams);
}

void UdpSocketHandler::closeSocket()
{
    qDebug() << "Closing SocketHandler";
    mUdpSocket->abort();
    mUdpSocket->close();
    delete mUdpSocket;
    mUdpSocket = nullptr;
}

/**
 * This function will run when theQUdpSocket send the readyRead signal
 * It will find and remove the streamId from the datagram, then do the same with
 * a single byte which will let us know if the datagram is audio or video.
 * The function will use the streamId to send the datagram to the correct buffer/playbackhandler
 */
void UdpSocketHandler::readPendingDatagrams()
{
    while (mUdpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = mUdpSocket->receiveDatagram();
        if(datagram.senderAddress().toIPv4Address() != mAddress.toIPv4Address())
        {
            continue;
        }
        QByteArray data = datagram.data();

        //To allow debugging on localhost. We added the operations the server performs on incoming datagrams before sending them to a client
       /* if(mAddress.toIPv4Address() != QHostAddress("46.250.220.57").toIPv4Address() && mAddress.toIPv4Address() != QHostAddress("213.162.241.177").toIPv4Address())
        {
            //roomId is the first x bytes, then streamId
            int roomIdLength = data[0];
            data.remove(0,1);

            //Finds the roomId header, stores it and removes it from the datagram
            QByteArray roomIdArray = QByteArray(data, roomIdLength);
            QString roomId(roomIdArray);
            data.remove(0, roomIdLength);
        }*/

        const int streamIdLength = data[0];
        data.remove(0,1);

        //Finds the streamId header, stores it and removes it;
        QByteArray streamIdArray = QByteArray(data, streamIdLength);
        QString streamId(streamIdArray);
        data.remove(0, streamIdLength);

        const int index = findStreamIdIndex(streamId);
        if(index < 0)
        {
            continue;
        }

        //Checks the first byte in the datagram to determine if the datagram is audio or video
        const int audioOrVideoInt = data[0];
        data.remove(0, 1);

        if(audioOrVideoInt == 0)
        {
            mInputStreamHandler->getAudioMutexVector()[index]->lock();
            mInputStreamHandler->getAudioBufferVector()[index]->append(data);
            mInputStreamHandler->getAudioMutexVector()[index]->unlock();
            if(!mInputStreamHandler->getAudioPlaybackStartedVector()[index] && mInputStreamHandler->getAudioBufferVector()[index]->size() >= (mBufferSize / 8))
            {
                *mInputStreamHandler->getAudioFutures().at(index) = QtConcurrent::run(mInputStreamHandler->getAudioPlaybackHandlerVector()[index], &AudioPlaybackHandler::start);
                qDebug() << "Starting AudioPlaybackHandler for streamId: " << streamId;
                mInputStreamHandler->getAudioPlaybackStartedVector()[index] = true;
            }
            //qDebug() << "audio buffer size " << mInputStreamHandler->mAudioBufferVector[index]->size() << "after signal: " << signalCount;
        }
        else if (audioOrVideoInt == 1)
        {
            mInputStreamHandler->getVideoMutexVector()[index]->lock();

            mInputStreamHandler->getVideoBufferVector()[index]->append(data);

            mInputStreamHandler->getVideoMutexVector()[index]->unlock();
            if(!mInputStreamHandler->getVideoPlaybackStartedVector()[index] && mInputStreamHandler->getVideoBufferVector()[index]->size() >= mBufferSize)
            {
                //qDebug() << "Buffer: " << (*mInputStreamHandler->mVideoBufferVector[index]);
                *mInputStreamHandler->getVideoFutures().at(index) = QtConcurrent::run(mInputStreamHandler->getVideoPlaybackHandlerVector()[index], &VideoPlaybackHandler::start);
                qDebug() << "STARTING VIDEOPLAYBACKHANDLER!!!";
                mInputStreamHandler->getVideoPlaybackStartedVector()[index] = true;
            }
            //qDebug() << "video buffer size " << mInputStreamHandler->mVideoBufferVector[index]->size() << "after signal: " << signalCount;
        }
        else
        {
            qDebug() << "UDP Header byte was not 1 or 0 in socketHandler, stopping program";
            exit(-1);
        }
    }

    signalCount++;
}

/**
 * When recieving a UDP datagram, we need to know who owns the stream.
 * The index will let readPendingDatagrams know which buffer, mutex and playbackhandler to use for both audio and video.
 * @param streamId QString to find in mStreamIdVector
 * @return int index where streamId was found
 */
int UdpSocketHandler::findStreamIdIndex(QString streamId)
{
    if(mInputStreamHandler->getStreamIdVector().size() >= 1)
    {
        for(size_t i = 0; i < mInputStreamHandler->getStreamIdVector().size(); i++)
        {
            if(QString::compare(streamId, mInputStreamHandler->getStreamIdVector()[i], Qt::CaseSensitive) == 0)
            {
                return i;
            }
        }
    }

    //TODO handle this?
    qDebug() << Q_FUNC_INFO << " failed to find streamId: " << streamId;
    qDebug() << "This means the server is sending datagrams to people it should not";
    qDebug() << "StreamIdVector: " << mInputStreamHandler->getStreamIdVector();
    return -1;
}
/**
 * Send the QByteArray through the current udpSocket,
 * if the size is larger than 512, it will be divided into smaller arrays.
 * The function will also prepend streamId and roomId from the SessionHandler
 * to the arrays.
 * @param arr QByteArray to be sent
 * @return Error code (0 if successful)
 */
int UdpSocketHandler::sendDatagram(QByteArray arr)
{
    int datagramMaxSize = 2*512;
    if(mUdpSocket == nullptr)
    {
        return AVERROR_EXIT;
    }
    int ret = 0;
    /*
     * In order for the dividing to work, we need to remove the audioOrVideo byte
     * at the start of the arr, and prepend it to the smaller arrays
    */
    qDebug() << arr.size();

    //Creats a new QByteArray from the first byte in arr, which should be the audioOrVideo byte.
    //Then it removes the byte from arr
    QByteArray arrToPrepend = QByteArray(arr, 1);
    arr.remove(0, 1);

    //Puts the streamId and its size at the front of the array,
    //so the server knows where to send the stream
    arrToPrepend.prepend(mStreamId.toLocal8Bit().data());
    arrToPrepend.prepend(mStreamId.size());

    //Puts the roomId and its size at the front of the array
    arrToPrepend.prepend(mRoomId.toLocal8Bit().data());
    arrToPrepend.prepend(mRoomId.size());

    while(arr.size() > 0)
    {
        if(arr.size() > (datagramMaxSize - arrToPrepend.size()))
        {
            /*
             * Creates a deep copy of x bytes in arr.
             * Prepends the QByteArray containing roomId, streamId, audioOrVideoByte.
             * Then removes x bytes from arr.
             */
            QByteArray temp = QByteArray(arr, (datagramMaxSize - arrToPrepend.size()));
            temp.prepend(arrToPrepend);
            arr.remove(0, (datagramMaxSize - arrToPrepend.size()));
            ret += mUdpSocket->writeDatagram(temp, temp.size(), mAddress, 1336);
        }
        else
        {
            arr.prepend(arrToPrepend);
            ret += mUdpSocket->writeDatagram(arr, arr.size(), mAddress, 1336);
            break;
        }

        if(ret < 0)
        {
            qDebug() << mUdpSocket->error();
            qDebug() << ret;
            break;
        }
        //if(ret >= arr.size()) break;
    }

    return ret;
}

