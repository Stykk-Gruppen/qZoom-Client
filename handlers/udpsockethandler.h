#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QNetworkDatagram>
#include <QProcess>
#include <mutex>
#include "handlers/imagehandler.h"
#include "handlers/videoplaybackhandler.h"
#include "handlers/audioplaybackhandler.h"

#include<stdio.h>	//printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>

#define SERVER "46.250.220.57"
#define BUFLEN 512	//Max length of buffer
#define PORT 1337	//The port on which to send data

class VideoPlaybackHandler;
class AudioPlaybackHandler;
class InputStreamHandler;
class UdpSocketHandler : public QObject
{
    Q_OBJECT
public:
    explicit UdpSocketHandler(int bufferSize, int port, InputStreamHandler* inputStreamHandler, QString streamId, QString roomId, QHostAddress address, QObject *parent = nullptr);
    void initSocket();
    void closeSocket();
    int sendDatagram(QByteArray arr);

public slots:
    void readPendingDatagrams();

private:
    int sendArray(QByteArray data);
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    void addStreamToVector(QString, int);
    //int findStreamIdIndex(QString);
    int mBufferSize;
    QTcpSocket* mTcpSocket;
    QUdpSocket* mUdpSocket;
    QHostAddress mAddress;
    int mPort;
    QString mRoomId;
    QString mStreamId;
    InputStreamHandler* mInputStreamHandler;
    uint signalCount = 0;
    struct mBufferAndLockStruct
    {
        QByteArray* buffer;
        std::mutex* writeLock;
    };
};

#endif // SOCKETHANDLER_H
