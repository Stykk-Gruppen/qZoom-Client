#include "tcpsockethandler.h"


TcpSocketHandler::TcpSocketHandler(InputStreamHandler* inputStreamHandler,  QString streamId, QString roomId, QString displayName, QHostAddress address, int port, QObject* parent): QObject(parent)
{
    mAddress = address;
    //mPort = port;
    mPort = 1338;
    qDebug() << mAddress;
    qDebug() << "Tcp port" << mPort;
    mInputStreamHandler = inputStreamHandler;
    mStreamId = streamId;
    mRoomId = roomId;
    mDisplayName = displayName;
}



void TcpSocketHandler::init()
{
    mSocket = new QTcpSocket(this);
    /*connect(mSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(mSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
*/

    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(mSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));

    mSocket->connectToHost(mAddress, mPort);
    //qDebug() << "After ConnectToHost, addr: " << mAddress << " port: " << mPort;
    if(!mSocket->waitForConnected(3000))
    {
        qDebug() << "TcpSocketError: " << mSocket->errorString();
    }
}

void TcpSocketHandler::close()
{
    qDebug() << "Closing TcpSocketHandler";
    mSocket->close();
    delete mSocket;
}

void TcpSocketHandler::readyRead()
{
    qDebug() << "TcpSocket reading incoming message";

    //qDebug() << mSocket->readAll();
    QByteArray data = mSocket->readAll();

    qDebug() << "Data received: " << data;


    int code = data[0];
    data.remove(0, 1);
    switch(code)
    {
    case VIDEO_HEADER:
    {
        //HEADER
        int numOfHeaders = data[0];
        qDebug() << "number of headers recieved from server: " << numOfHeaders;
        data.remove(0, 1);
        //QString data(reply);


        //qDebug() << "DataString: " << data;
        //qDebug() << reply.indexOf(27);
        for(int i = 0; i < numOfHeaders; i++)
        {
            //qDebug() << "Data: " << data;
            QByteArray temp = QByteArray(data, data.indexOf(27));
            //qDebug() << "Temp: " << temp;
            mInputStreamHandler->handleHeader(temp);
            //qDebug() << "Data before remove endline: " << data;
            data.remove(0, (data.indexOf(27) + 1));
            //qDebug() << "Data after remove endline: " << data;
        }
        //mSocket->write("0");
        break;
    }
    case REMOVE_PARTICIPANT:
    {
        //REmove the user with this streamId
        qDebug() << "About to remove this user: " << data;
        int streamIdLength = data[0];
        data.remove(0, 1);
        QByteArray streamIdArray = QByteArray(data, streamIdLength);
        QString streamId(streamIdArray);
        data.remove(0, streamIdLength);

        mInputStreamHandler->removeStream(streamId);
        break;
    }
    case NEW_DISPLAY_NAME:
    {
        //Update Participant displayName
        qDebug() << "Updating display name for user: " << data;
        int displayNameLength = data[0];
        data.remove(0, 1);
        QByteArray displayNameArray = QByteArray(data, displayNameLength);
        QString displayName(displayNameArray);
        data.remove(0, displayNameLength);

        int streamIdLength = data[0];
        data.remove(0, 1);
        QByteArray streamIdArray = QByteArray(data, streamIdLength);
        QString streamId(streamIdArray);
        data.remove(0, streamIdLength);

        mInputStreamHandler->updateParticipantDisplayName(streamId, displayName);
        break;
    }
    case VIDEO_DISABLED:
    {
        qDebug() << "Setting user to muted for: " << data;
        int streamIdLength = data[0];
        data.remove(0, 1);
        QByteArray streamIdArray = QByteArray(data, streamIdLength);
        QString streamId(streamIdArray);
        data.remove(0, streamIdLength);

        mInputStreamHandler->setPeerToVideoDisabled(streamId);
        break;
    }
    case AUDIO_DISABLED:
    {
        qDebug() << "Setting user to muted for: " << data;
        int streamIdLength = data[0];
        data.remove(0, 1);
        QByteArray streamIdArray = QByteArray(data, streamIdLength);
        QString streamId(streamIdArray);
        data.remove(0, streamIdLength);

        mInputStreamHandler->setPeerToAudioDisabled(streamId);
        break;
    }
    default:
        qDebug() << "Dont understand the code sent in beginning of tcp mesasge";
    };
}


//Send header to server, and receive headers from other participants back
void TcpSocketHandler::writeHeader()
{
    //qDebug() << "About to write header";
    //mSocket->connectToHost(mAddress, mPort);

    myHeader.prepend(mStreamId.toLocal8Bit().data());
    myHeader.prepend(mStreamId.size());

    myHeader.prepend(mDisplayName.toLocal8Bit().data());
    myHeader.prepend(mDisplayName.size());

    //Puts the roomId and its size at the front of the array
    myHeader.prepend(mRoomId.toLocal8Bit().data());
    myHeader.prepend(mRoomId.size());

    qDebug() << "My Header: " << myHeader.length() << "\n" << myHeader;

    mSocket->write(myHeader);
    myHeader.clear();
    //mSocket->write("HEAD / HTTP/1.0\r\n\r\n\r\n\r\n");
    //while (mSocket->waitForReadyRead(3000));

    /*QByteArray reply = mSocket->readAll();

    //qDebug() << "Reply from Server: \n" << reply;
    if(reply.size() <= 0)
    {
        qDebug() << "Reply from tcp request was empty or timeout, should not happen @ " << Q_FUNC_INFO;
        return;
    }
    else if(reply.size()==1)
    {
        int returnCode = reply[0];
        switch(returnCode)
        {
        case mTcpReturnValues::SESSION_STARTED:
            //TODO how to handle session started and nothing returned from server
            qDebug() << "Server responded with session started";
            return;
        case mTcpReturnValues::ROOM_ID_NOT_FOUND:
            //TODO handle wrong roomId
            qDebug() << "Server did not find roomId";
            return;
        case mTcpReturnValues::STREAM_ID_NOT_FOUND:
            qDebug() << "Server did not find streamId";
            //TODO handle wrong streamId
            return;
        default:
            qDebug() << "Unkown return code from tcp server @ " << Q_FUNC_INFO;
            exit(-1);
        }
    }*/

    /*int numOfHeaders = reply[0];
    qDebug() << "number of headers recieved from server: " << numOfHeaders;
    reply.remove(0,1);
    //QString data(reply);


    //qDebug() << "DataString: " << data;
    //qDebug() << reply.indexOf(27);


    for(int i = 0; i < numOfHeaders; i++)
    {
        QByteArray temp = QByteArray(reply, reply.indexOf(27));
        //qDebug() << "Temp: " << temp;
        mInputStreamHandler->handleHeader(temp);
        reply.remove(0, reply.indexOf(27));
    }
    */

    //mSocket->write("0");

}

void TcpSocketHandler::sendChangedDisplayNameSignal()
{
    //QByteArray header = QString("DISPLAY_NAME_UPDATE").toUtf8();
    QByteArray header = QByteArray::number(NEW_DISPLAY_NAME);

    header.prepend(mStreamId.toLocal8Bit().data());
    header.prepend(mStreamId.size());

    header.prepend(mDisplayName.toLocal8Bit().data());
    header.prepend(mDisplayName.size());

    //Puts the roomId and its size at the front of the array
    header.prepend(mRoomId.toLocal8Bit().data());
    header.prepend(mRoomId.size());

    qDebug() << "My Header: " << header.length() << "\n" << header;

    mSocket->write(header);
    header.clear();
}

void TcpSocketHandler::sendDisabledVideoSignal()
{
    //QByteArray header = QString("VIDEO_DISABLED").toUtf8();
    QByteArray header = QByteArray::number(VIDEO_DISABLED);

    header.prepend(mStreamId.toLocal8Bit().data());
    header.prepend(mStreamId.size());

    //Don't really need this, but removing it would make the server parser struggle.
    header.prepend(mDisplayName.toLocal8Bit().data());
    header.prepend(mDisplayName.size());

    //Puts the roomId and its size at the front of the array
    header.prepend(mRoomId.toLocal8Bit().data());
    header.prepend(mRoomId.size());

    qDebug() << "My Header: " << header.length() << "\n" << header;

    mSocket->write(header);
    header.clear();
}

void TcpSocketHandler::sendDisabledAudioSignal()
{
    //QByteArray header = QString("AUDIO_DISABLED").toUtf8();
    QByteArray header = QByteArray::number(AUDIO_DISABLED);

    header.prepend(mStreamId.toLocal8Bit().data());
    header.prepend(mStreamId.size());

    //Don't really need this, but removing it would make the server parser struggle.
    header.prepend(mDisplayName.toLocal8Bit().data());
    header.prepend(mDisplayName.size());

    //Puts the roomId and its size at the front of the array
    header.prepend(mRoomId.toLocal8Bit().data());
    header.prepend(mRoomId.size());

    qDebug() << "My Header: " << header.length() << "\n" << header;

    mSocket->write(header);
    header.clear();
}


void TcpSocketHandler::connected()
{
    qDebug() << "Tcp socket connected to " << mAddress << "on port " << mPort;
    qDebug() << "TCP Request: " << mRequest;
    //mSocket->write(mRequest);
}

void TcpSocketHandler::disconnected()
{
    qDebug() << "TCPSocket disconnected";
}

void TcpSocketHandler::bytesWritten(qint64 bytes)
{
    qDebug() << "Tcpsocket wrote " << bytes << " bytes";
    mBytesWritten = bytes;
}

void TcpSocketHandler::wait()
{
    while (mSocket->waitForReadyRead(1000));
}

int TcpSocketHandler::getBytesWritten()
{
    return mBytesWritten;
}

QByteArray TcpSocketHandler::getReply()
{
    return mReply;
}

bool TcpSocketHandler::isReady()
{
    return mReady;
}

void TcpSocketHandler::updateDisplayName(QString displayName)
{
    mDisplayName = displayName;
}
