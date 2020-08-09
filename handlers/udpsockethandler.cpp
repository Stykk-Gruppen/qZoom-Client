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

    //char buf[BUFLEN];
    //char *message = "test message very many bytes to send with this message";

    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        exit(1);
    }

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);

    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
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

        const int index = mInputStreamHandler->findStreamIdIndex(streamId);
        if(index < 0)
        {
            continue;
        }

        //Checks the first byte in the datagram to determine if the datagram is audio or video
        const int audioOrVideoInt = data[0];
        data.remove(0, 1);

        if(audioOrVideoInt == 0)
        {
            mInputStreamHandler->lockAudioMutex(index);
            mInputStreamHandler->appendToAudioBuffer(index, data);
            mInputStreamHandler->unlockAudioMutex(index);
            if(!mInputStreamHandler->audioPlaybackStarted(index) && mInputStreamHandler->getAudioBufferSize(index) >= (mBufferSize / 8))
            {
                *mInputStreamHandler->getAudioFutures(index) = QtConcurrent::run(mInputStreamHandler->getAudioPlaybackHandler(index), &AudioPlaybackHandler::start);
                qDebug() << "Starting AudioPlaybackHandler for streamId: " << streamId;
                mInputStreamHandler->setAudioPlaybackStarted(index, true);
            }
            //qDebug() << "audio buffer size " << mInputStreamHandler->mAudioBufferVector[index]->size() << "after signal: " << signalCount;
        }
        else if (audioOrVideoInt == 1)
        {
            mInputStreamHandler->lockVideoMutex(index);
            mInputStreamHandler->appendToVideoBuffer(index, data);
            mInputStreamHandler->unlockVideoMutex(index);
            if(!mInputStreamHandler->videoPlaybackStarted(index) && mInputStreamHandler->getVideoBufferSize(index) >= mBufferSize)
            {
                //qDebug() << "Buffer: " << (*mInputStreamHandler->mVideoBufferVector[index]);
                *mInputStreamHandler->getVideoFutures(index) = QtConcurrent::run(mInputStreamHandler->getVideoPlaybackHandler(index), &VideoPlaybackHandler::start);
                qDebug() << "STARTING VIDEOPLAYBACKHANDLER!!!";
                mInputStreamHandler->setVideoPlaybackStarted(index, true);
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
/*
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
*/
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
    //qDebug() << arr.size();

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
            ret += sendArray(temp);
        }
        else
        {
            /* We do not remove anything from arr if it is
             * smaller than (datagramMaxSize - arrToPrepend.size()),
             * so we need the break to leave the while loop
            */
            arr.prepend(arrToPrepend);
            ret += sendArray(arr);
            break;
        }

        if(ret < 0)
        {
            //TODO this should also print c++ socket error, could end up here with
            //mUdpSocket->error(); not printing anything usefull
            qDebug() << mUdpSocket->error();
            qDebug() << ret;
            break;
        }

    }

    return ret;
}
/**
 * On most routers, if we do not "open" the port by sending a datagram with QAbstractSocket
 * it will block incoming datagrams, and the program will recieve nothing
 * @param data QByteArray containing data to be sent
 * @return int with how many bytes sent, or the error code
 */
int UdpSocketHandler::sendArray(QByteArray data)
{
    static bool openPortWithQUDP = false;
    if(openPortWithQUDP)
    {
        //Returns number of bytes sent, or -1 or error
        return sendto(s, data, data.size(), 0 , (struct sockaddr *) &si_other, slen);
    }
    else
    {
        openPortWithQUDP = true;
        return mUdpSocket->writeDatagram(data, data.size(), mAddress, mPort);
    }
}

