#include "tcpsockethandler.h"

/**
 * @brief TcpSocketHandler::TcpSocketHandler
 * @param inputStreamHandler
 * @param streamId
 * @param roomId
 * @param displayName
 * @param address
 * @param port
 * @param parent
 */
TcpSocketHandler::TcpSocketHandler(InputStreamHandler* inputStreamHandler, QString streamId,
                                   QString roomId, QString displayName,
                                   QHostAddress address, int port,
                                   QObject* parent): QObject(parent)
{
    mAddress = address;
    mPort = port;
    mInputStreamHandler = inputStreamHandler;
    mStreamId = streamId;
    mRoomId = roomId;
    mDisplayName = displayName;
}

/**
 * @brief TcpSocketHandler::~TcpSocketHandler
 */
TcpSocketHandler::~TcpSocketHandler()
{
    delete mSocket;
    mSocket = nullptr;
}

/**
 * @brief TcpSocketHandler::close
 */
void TcpSocketHandler::close()
{
    qDebug() << "Closing TcpSocketHandler";
    mSocket->close();
}

/**
 * @brief TcpSocketHandler::init
 * @return
 */
int TcpSocketHandler::init()
{
    mSocket = new QTcpSocket(this);

    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(mSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));

    mSocket->connectToHost(mAddress, mPort);
    if(!mSocket->waitForConnected(3000))
    {
        qDebug() << "TcpSocketError: " << mSocket->errorString();
        errorHandler->giveErrorDialog("Could not connect to server");
        qDebug() << mAddress << mPort;
        return -1;
    }
    return 0;
}

/**
 * @brief TcpSocketHandler::isOpen
 * @return
 */
bool TcpSocketHandler::isOpen() const
{
    qDebug() << "isOpen is returning: " << (mSocket != nullptr);
    return mSocket != nullptr;
}

/**
 * @brief TcpSocketHandler::readyRead
 */
void TcpSocketHandler::readyRead()
{
    qDebug() << "TcpSocket reading incoming message";

    QByteArray data = mSocket->readAll();

    qDebug() << "Data received: " << data;

    //To prepare our router to recieve datagrams, we need to send a empty one to the server.
    emit sendDummyDatagram();

    const int code = data[0];
    data.remove(0, 1);
    switch(code)
    {
    case VIDEO_HEADER:
    {
        const int numOfHeaders = data[0];
        qDebug() << "number of headers recieved from server: " << numOfHeaders;
        data.remove(0, 1);
        for (int i = 0; i < numOfHeaders; i++)
        {
            QByteArray temp = QByteArray(data, data.indexOf(27));
            mInputStreamHandler->handleHeader(temp);
            data.remove(0, (data.indexOf(27) + 1));
        }
        break;
    }
    case REMOVE_PARTICIPANT:
    {
        //Remove the user with this streamId
        data.remove(0, 1);
        qDebug() << "About to remove this user: " << data;
        const int streamIdLength = data[0];
        data.remove(0, 1);
        const QByteArray streamIdArray = QByteArray(data, streamIdLength);
        const QString streamId(streamIdArray);
        data.remove(0, streamIdLength);

        mInputStreamHandler->removeStream(streamId);
        break;
    }
    case NEW_DISPLAY_NAME:
    {
        //Update Participant displayName
        data.remove(0, 1);
        qDebug() << "Updating display name for user: " << data;
        const int displayNameLength = data[0];
        data.remove(0, 1);
        const QByteArray displayNameArray = QByteArray(data, displayNameLength);
        const QString displayName(displayNameArray);
        data.remove(0, displayNameLength);

        const int streamIdLength = data[0];
        data.remove(0, 1);
        const QByteArray streamIdArray = QByteArray(data, streamIdLength);
        const QString streamId(streamIdArray);
        data.remove(0, streamIdLength);

        mInputStreamHandler->updateParticipantDisplayName(streamId, displayName);
        break;
    }
    case VIDEO_DISABLED:
    {
        data.remove(0, 1);
        qDebug() << "Setting user's video to disabled for: " << data;
        const int streamIdLength = data[0];
        data.remove(0, 1);
        const QByteArray streamIdArray = QByteArray(data, streamIdLength);
        const QString streamId(streamIdArray);
        data.remove(0, streamIdLength);

        mInputStreamHandler->setPeerToVideoDisabled(streamId);
        break;
    }
    case AUDIO_DISABLED:
    {
        data.remove(0, 1);
        qDebug() << "Setting user to muted for: " << data;
        const int streamIdLength = data[0];
        data.remove(0, 1);
        const QByteArray streamIdArray = QByteArray(data, streamIdLength);
        const QString streamId(streamIdArray);
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

/**
 * @brief TcpSocketHandler::getSocket
 * @return
 */
QTcpSocket *TcpSocketHandler::getSocket()
{
    return mSocket;
}

/**
 * @brief TcpSocketHandler::sendVideoHeader
 */
void TcpSocketHandler::sendVideoHeader()
{
    mHeader.prepend(VIDEO_HEADER);
    prependDefaultHeader(mHeader);

    qDebug() << "My Header: " << mHeader.length() << "\n" << mHeader;

    mSocket->write(mHeader);
    qDebug() << "after write";
    mHeader.clear();
}

/**
 * @brief TcpSocketHandler::sendChangedDisplayNameSignal
 */
void TcpSocketHandler::sendChangedDisplayNameSignal()
{
    QByteArray header;
    header.append(NEW_DISPLAY_NAME);

    prependDefaultHeader(header);

    qDebug() << "My Header: " << header.length() << "\n" << header;

    mSocket->write(header);
    header.clear();
}

/**
 * @brief TcpSocketHandler::sendDisabledVideoSignal
 */
void TcpSocketHandler::sendDisabledVideoSignal()
{
    QByteArray header;
    header.append(VIDEO_DISABLED);

    prependDefaultHeader(header);

    qDebug() << "My Header: " << header.length() << "\n" << header;

    mSocket->write(header);
    mSocket->waitForBytesWritten();
}

/**
 * @brief TcpSocketHandler::sendDisabledAudioSignal
 */
void TcpSocketHandler::sendDisabledAudioSignal()
{
    QByteArray header;
    header.append(AUDIO_DISABLED);

    prependDefaultHeader(header);

    qDebug() << "My Header: " << header.length() << "\n" << header;

    mSocket->write(header);
    header.clear();
}

/**
 * @brief TcpSocketHandler::sendKickParticipantSignal
 * @param streamId
 */
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

/**
 * @brief TcpSocketHandler::prependDefaultHeader
 * @param data
 */
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

/**
 * @brief TcpSocketHandler::appendToHeader
 * @param data
 */
void TcpSocketHandler::appendToHeader(const QByteArray &data)
{
    mHeader.append(data);
}

/**
 * @brief TcpSocketHandler::bytesWritten
 * @param bytes
 */
void TcpSocketHandler::bytesWritten(qint64 bytes)
{
    qDebug() << "Tcpsocket wrote " << bytes << " bytes";
    mBytesWritten = bytes;
}

/**
 * @brief TcpSocketHandler::updateDisplayName
 * @param displayName
 */
void TcpSocketHandler::updateDisplayName(const QString& displayName)
{
    mDisplayName = displayName;
}
