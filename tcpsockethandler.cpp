#include "tcpsockethandler.h"


TcpSocketHandler::TcpSocketHandler(InputStreamHandler* inputStreamHandler, SessionHandler* sessionHandler, QHostAddress address, int port, QObject* parent): QObject(parent)
{
    mAddress = address;
    mPort = port;
    qDebug() << mAddress;
    qDebug() << "Tcp port" << mPort;
    mInputStreamHandler = inputStreamHandler;
    mSessionHandler = sessionHandler;
}

void TcpSocketHandler::init()
{
    mSocket = new QTcpSocket(this);
    /*connect(mSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(mSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
*/
    connect(mSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));







}
//Send header to server, and receive headers from other participants back
void TcpSocketHandler::writeHeader()
{
    static bool firstRound = true;
    //mSocket->connectToHost(mAddress, mPort);
    mSocket->connectToHost(mAddress, mPort);
    qDebug() << "After ConnectToHost";
    if(!mSocket->waitForConnected(3000))
    {
        qDebug() << "TcpSocketError: " << mSocket->errorString();
    }

    if(firstRound)
    {
        QString streamId = mSessionHandler->getUser()->getStreamId();
        QString roomId = mSessionHandler->getRoomId();

        myHeader.prepend(streamId.toLocal8Bit().data());
        myHeader.prepend(streamId.size());

        //Puts the roomId and its size at the front of the array
        myHeader.prepend(roomId.toLocal8Bit().data());
        myHeader.prepend(roomId.size());

        firstRound = false;
    }



    //qDebug() << "My Header: " << myHeader.length() << "\n" << myHeader;


    mSocket->write(myHeader);
    //mSocket->write("HEAD / HTTP/1.0\r\n\r\n\r\n\r\n");
    while (mSocket->waitForReadyRead(3000));

    QByteArray reply = mSocket->readAll();
    //qDebug() << "Reply from Server: \n" << reply;

    int numOfHeaders = reply[0];
    qDebug() << numOfHeaders;
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

    //mSocket->write("0");

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

void TcpSocketHandler::readyRead()
{
    qDebug() << "TcpSocket reading...";

    //qDebug() << mSocket->readAll();
    mReply = mSocket->readAll();
    mReady = true;
}

QByteArray TcpSocketHandler::getReply()
{
    return mReply;
}

bool TcpSocketHandler::isReady()
{
    return mReady;
}
