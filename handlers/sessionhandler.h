#ifndef SESSIONHANDLER_H
#define SESSIONHANDLER_H

#include <QObject>
#include "handlers/userhandler.h"
#include <QHostInfo>
#include <QHostAddress>
#include <QNetworkInterface>
#include "imagehandler.h"
#include "inputstreamhandler.h"
#include "tcpsockethandler.h"
#include "handlers/imagehandler.h"
#include "handlers/inputstreamhandler.h"
#include "handlers/udpsockethandler.h"
#include "handlers/tcpsockethandler.h"
#include "handlers/outputstreamhandler.h"
#include "core/servertcpqueries.h"

class SessionHandler : public QObject
{
    Q_OBJECT
public:
    explicit SessionHandler(ServerTcpQueries* _mServerTcpQueries, UserHandler* _user,
                            ImageHandler* imageHandler,
                            Settings* settings, int bufferSize,
                            QHostAddress address, int _portNumberTCP,
                            int _portNumerUDP,
                            QObject *parent = nullptr);
    Q_INVOKABLE void updateDisplayName();
    Q_INVOKABLE void disableVideo();
    Q_INVOKABLE void disableAudio();
    Q_INVOKABLE bool joinSession(const QString& roomId, const QString& roomPassword);
    Q_INVOKABLE bool createSession(const QString& roomId, const QString& roomPassword);
    Q_INVOKABLE bool leaveSession();
    Q_INVOKABLE bool isGuest() const;
    Q_INVOKABLE bool enableVideo();
    Q_INVOKABLE bool enableAudio();
    Q_INVOKABLE bool checkVideoEnabled() const;
    Q_INVOKABLE bool checkAudioEnabled() const;
    Q_INVOKABLE bool enableScreenShare();
    Q_INVOKABLE QString getRoomId() const;
    Q_INVOKABLE QString getRoomPassword() const;
    Q_INVOKABLE QString getRoomHostUsername() const;
    Q_INVOKABLE QVariantList getAudioInputDevices() const;

private:
    ServerTcpQueries* mServerTcpQueries;
    void addUser();
    int initOtherStuff();
    void closeOtherStuff();
    void getUserRoom();
    void setDefaultRoomID();
    bool addGuestUserToDatabase();
    bool mUserHasRoom;
    bool mSessionIsActive;
    int mBufferSize;
    int mPortNumberTCP;
    int mPortNumberUDP;
    QString mRoomId;
    QString mRoomPassword;
    QString mRoomHostUsername;
    QString mIpAddress;
    QHostAddress mAddress;
    UserHandler* mUser;
    Settings* mSettings;
    OutputStreamHandler* mOutputStreamHandler;
    ImageHandler* mImageHandler;
    InputStreamHandler* mInputStreamHandler;
    UdpSocketHandler* mUdpSocketHandler;
    TcpSocketHandler* mTcpSocketHandler;

signals:

};

#endif // SESSIONHANDLER_H
