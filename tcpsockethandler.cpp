#include "tcpsockethandler.h"


TcpSocketHandler::TcpSocketHandler(QHostAddress address, QByteArray request, int port, QObject* parent): QObject(parent)
{
    mReady = false;
    mRequest = request;
    mAddress = address;
    mPort = port;
    //mPort = 1338;
    //mPort = 80;
    init();
}

void TcpSocketHandler::init()
{
    mSocket = new QTcpSocket(this);
    connect(mSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(mSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(mSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));

    qDebug() << "Connecting tcp socket..";

    mSocket->connectToHost(mAddress, mPort);

    if(!mSocket->waitForConnected(3000))
    {
        qDebug() << "TcpSocketError: " << mSocket->errorString();
    }
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
