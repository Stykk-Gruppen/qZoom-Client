#include "tcpsockethandler.h"


TcpSocketHandler::TcpSocketHandler(InputStreamHandler* inputStreamHandler, QString streamId,
                                   QString roomId, QString displayName,
                                   QHostAddress address, int port,
                                   QObject* parent): QObject(parent)
{
    mAddress = address;
    mPort = port;
    qDebug() << mAddress;
    qDebug() << "Tcp port" << mPort;
    mInputStreamHandler = inputStreamHandler;
    mStreamId = streamId;
    mRoomId = roomId;
    mDisplayName = displayName;
}

TcpSocketHandler::~TcpSocketHandler()
{

}

int TcpSocketHandler::init()
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
        errorHandler->giveErrorDialog("Could not connect to server");
        return -1;
    }
    return 0;
}

bool TcpSocketHandler::isOpen() const
{
    qDebug() << "isOpen is returning: " << (mSocket != nullptr);
    return mSocket != nullptr;
}

void TcpSocketHandler::close()
{
    qDebug() << "Closing TcpSocketHandler";
    mSocket->close();
    delete mSocket;
    mSocket = nullptr;
}

void TcpSocketHandler::readyRead()
{
    qDebug() << "TcpSocket reading incoming message";

    //qDebug() << mSocket->readAll();
    QByteArray data = mSocket->readAll();

    qDebug() << "Data received: " << data;

    //To prepare our router to recieve datagrams, we need to send a empty one to the server.
    emit sendDummyDatagram();

    int code = data[0];
    data.remove(0, 1);
    switch(code)
    {
    case VIDEO_HEADER:
    {

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
        data.remove(0, 1);
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
        data.remove(0, 1);
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
        data.remove(0, 1);
        qDebug() << "Setting user's video to disabled for: " << data;
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
        data.remove(0, 1);
        qDebug() << "Setting user to muted for: " << data;
        int streamIdLength = data[0];
        data.remove(0, 1);
        QByteArray streamIdArray = QByteArray(data, streamIdLength);
        QString streamId(streamIdArray);
        data.remove(0, streamIdLength);

        mInputStreamHandler->setPeerToAudioDisabled(streamId);
        break;
    }
    case KICK_PARTICIPANT:
    {
        qDebug() << "You have been kicked by the host!";
        mInputStreamHandler->kickYourself();
        break;
    }
    default:
        qDebug() << "Dont understand the code sent in beginning of tcp mesasge";
    };
}

QTcpSocket *TcpSocketHandler::getSocket()
{
    return mSocket;
}

//Send header to server, and receive headers from other participants back
void TcpSocketHandler::writeHeader()
{
    prependDefaultHeader(mHeader);

    qDebug() << "My Header: " << mHeader.length() << "\n" << mHeader;

    mSocket->write(mHeader);
    mHeader.clear();
}

void TcpSocketHandler::sendChangedDisplayNameSignal()
{
    QByteArray header;
    header.append(NEW_DISPLAY_NAME);

    prependDefaultHeader(header);

    qDebug() << "My Header: " << header.length() << "\n" << header;

    mSocket->write(header);
    header.clear();
}

void TcpSocketHandler::sendDisabledVideoSignal()
{
    //QByteArray header = QString("VIDEO_DISABLED").toUtf8();
    QByteArray header;
    header.append(VIDEO_DISABLED);

    prependDefaultHeader(header);

    qDebug() << "My Header: " << header.length() << "\n" << header;

    mSocket->write(header);
    header.clear();
}

void TcpSocketHandler::sendDisabledAudioSignal()
{
    //QByteArray header = QString("AUDIO_DISABLED").toUtf8();
    QByteArray header;
    header.append(AUDIO_DISABLED);

    prependDefaultHeader(header);

    qDebug() << "My Header: " << header.length() << "\n" << header;

    mSocket->write(header);
    header.clear();
}

void TcpSocketHandler::sendKickParticipantSignal(const QString& streamId)
{
    QByteArray header;

    header.append(KICK_PARTICIPANT);
    header.append(streamId.size());
    header.append(streamId.toLocal8Bit().data());

    prependDefaultHeader(header);

    qDebug() << "My Header: " << header.length() << "\n" << header;

    mSocket->write(header);
    header.clear();
}

void TcpSocketHandler::prependDefaultHeader(QByteArray& data) const
{
    data.prepend(mStreamId.toLocal8Bit().data());
    data.prepend(mStreamId.size());

    //Don't really need this all the time, but removing it would make the server parser struggle.
    data.prepend(mDisplayName.toLocal8Bit().data());
    data.prepend(mDisplayName.size());

    //Puts the roomId and its size at the front of the array
    data.prepend(mRoomId.toLocal8Bit().data());
    data.prepend(mRoomId.size());
}

void TcpSocketHandler::appendToHeader(const QByteArray &data)
{
    mHeader.append(data);
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

int TcpSocketHandler::getBytesWritten() const
{
    return mBytesWritten;
}

QByteArray TcpSocketHandler::getReply() const
{
    return mReply;
}

bool TcpSocketHandler::isReady() const
{
    return mReady;
}

void TcpSocketHandler::updateDisplayName(const QString& displayName)
{
    mDisplayName = displayName;
}
