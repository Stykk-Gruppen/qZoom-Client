#ifndef SESSIONHANDLER_H
#define SESSIONHANDLER_H

#include <QObject>
#include "core/database.h"
#include "handlers/userhandler.h"
#include <QHostInfo>
#include <QHostAddress>
#include <QNetworkInterface>
#include "imagehandler.h"
#include "inputstreamhandler.h"
#include "sockethandler.h"
#include "tcpserverhandler.h"
#include "tcpsockethandler.h"
#include "streamhandler.h"

class SessionHandler : public QObject
{
    Q_OBJECT
public:
    explicit SessionHandler(Database* _db, UserHandler* _user, ImageHandler* imageHandler, Settings* settings, int bufferSize, QHostAddress address, int port,  QObject *parent = nullptr);
    Q_INVOKABLE bool joinSession(QString _roomId, QString _roomPassword);
    Q_INVOKABLE bool createSession(QString _roomId, QString _roomPassword);
    Q_INVOKABLE bool leaveSession();
    Q_INVOKABLE bool isGuest();
    Q_INVOKABLE QString getRoomId();
    Q_INVOKABLE QString getRoomPassword();
    Q_INVOKABLE QString getRoomHostUsername();
    UserHandler* getUser();

    Q_INVOKABLE void enableVideo();
    Q_INVOKABLE void disableVideo();
    Q_INVOKABLE void enableAudio();

    Q_INVOKABLE void disableAudio();



private:
    void addUser();
    void initOtherStuff();
    void closeOtherStuff();
    QString mRoomId;
    QString mRoomPassword;
    QString mRoomHostUsername;
    QString mIpAddress;

    Database* mDb;
    UserHandler* mUser;
    bool mUserHasRoom;
    void getUserRoom();
    bool addGuestUserToDatabase();
    void setDefaultRoomID();

    int mBufferSize;
    int mPort;
    QHostAddress mAddress;

    Settings* mSettings;
    StreamHandler* mStreamHandler;
    ImageHandler* mImageHandler;
    InputStreamHandler* mInputStreamHandler;
    SocketHandler* mSocketHandler;
    TcpServerHandler* mTcpServerHandler;
    TcpSocketHandler* mTcpSocketHandler;




signals:

};

#endif // SESSIONHANDLER_H
