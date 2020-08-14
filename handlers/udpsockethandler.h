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
#include <QDateTime>
#include<stdio.h>	//printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>

class VideoPlaybackHandler;
class AudioPlaybackHandler;
class InputStreamHandler;
class UdpSocketHandler : public QObject
{
    Q_OBJECT
public:
    explicit UdpSocketHandler(int bufferSize, int port, InputStreamHandler* inputStreamHandler,
                              QString streamId, QString roomId, QHostAddress address, QObject *parent = nullptr);
    ~UdpSocketHandler();
    void initSocket();
    void closeSocket();
    int sendDatagram(QByteArray arr);

public slots:
    void readPendingDatagrams();
    void openPortHack();

private:
    void printBytesPerSecond(int bytes);
    int mCppUdpSocket;
    int slen = sizeof(si_other);
    int mBufferSize;
    int mPort;
    QTcpSocket* mTcpSocket;
    QUdpSocket* mUdpSocket;
    QHostAddress mAddress;
    QString mRoomId;
    QString mStreamId;
    InputStreamHandler* mInputStreamHandler;
    uint signalCount = 0;
    struct sockaddr_in si_other;
    struct mBufferAndLockStruct
    {
        QByteArray* buffer;
        std::mutex* writeLock;
    };
};

#endif // SOCKETHANDLER_H
