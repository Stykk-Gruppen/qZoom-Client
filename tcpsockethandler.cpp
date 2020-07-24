#include "tcpsockethandler.h"


TcpSocketHandler::TcpSocketHandler(InputStreamHandler* inputStreamHandler, QHostAddress address, int port, QObject* parent): QObject(parent)
{
    mAddress = address;
    mPort = port;
    qDebug() << "Tcp port" << mPort;
    mInputStreamHandler = inputStreamHandler;
}

void TcpSocketHandler::init()
{
    mSocket = new QTcpSocket(this);
    /*connect(mSocket, SIGNAL(connected()), this, SLOT(connected()));
    connect(mSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(mSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
*/







}
//Send header to server, and receive headers from other participants back
void TcpSocketHandler::writeHeader()
{
    //mSocket->connectToHost(mAddress, mPort);
    mSocket->connectToHost(mAddress, mPort);
    qDebug() << "After ConnectToHost";
    if(!mSocket->waitForConnected(3000))
    {
        qDebug() << "TcpSocketError: " << mSocket->errorString();
    }

    mSocket->write(myHeader);
    //mSocket->write("HEAD / HTTP/1.0\r\n\r\n\r\n\r\n");
    while (mSocket->waitForReadyRead(1000));

    QByteArray reply = mSocket->readAll();
    qDebug() << reply;

    int numOfHeaders = reply[0];
    reply.remove(0,1);

    QString data(reply);
    QStringList headers = data.split('\n');

    if(numOfHeaders != headers.size()) qDebug() << "Something strange occured with headers received";

    for(QString header : headers)
    {
        mInputStreamHandler->handleHeader(header.toLocal8Bit());
    }

    mSocket->write("0");

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
