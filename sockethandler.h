#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <QObject>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QProcess>

class SocketHandler : public QObject
{
    Q_OBJECT
public:
    explicit SocketHandler(QObject *parent = nullptr);
    void initSocket();

    QUdpSocket* udpSocket;
    int sendDatagram(QByteArray arr);
    QHostAddress address;
    int port;
    QByteArray mBuffer;
public slots:
    void readPendingDatagrams();
private:
    uint signalCount = 0;
    bool mPlaybackStarted = false;
signals:
    void startPlayback();

};

#endif // SOCKETHANDLER_H
