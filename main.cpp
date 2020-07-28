#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlContext>
#include <QCamera>
#include <QCameraInfo>
#include <QVideoProbe>
#include <QMediaPlayer>
#include <QMediaRecorder>
#include <QObject>
#include <QVideoFrame>
#include <QCameraImageCapture>
#include <QBuffer>
#include "videohandler.h"
#include "audiohandler.h"
#include <QCameraViewfinder>
#include <QVariant>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <QAudioInput>
#include <QFile>
#include "videohandler.h"
#include <QTimer>
#include "streamhandler.h"
#include "videoplaybackhandler.h"
#include "settings.h"
#include "imagehandler.h"
#include <QQuickView>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
#include <ao/ao.h>
}
#include <QtCore/QCoreApplication>
#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickView>
#include "inputstreamhandler.h"
#include "tcpsockethandler.h"
#include "audioplaybackhandler.h"
#include "handlers/sessionhandler.h"
#include "handlers/userhandler.h"
#include "core/database.h"


int main(int argc, char *argv[])
{
    QHostAddress address;
    address = QHostAddress::LocalHost;
    //address = QHostAddress("46.250.220.57"); //tarves.no
    //address = QHostAddress("158.36.165.235"); //Tarald
    //address = QHostAddress("92.220.136.246"); //Stian
    //address = QHostAddress("79.160.58.120"); //Kent
    //address = QHostAddress("213.162.241.177"); //KentServer


    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/view/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    //TcpSocketHandler* tcp = new TcpSocketHandler(QHostAddress("172.217.21.174"), "HEAD / HTTP/1.0\r\n\r\n\r\n\r\n", 80);
    //tcp->wait();
    //QString reply = tcp->getReply();


    //qDebug() << reply;
    QScopedPointer<Settings> settings(new Settings());


    //When buffer size is larger than 2k the server sends datagrams, but they do not arrive at the client (for video)
    int bufferSize = 8*1024;

    Database* databaseObject = new Database();
    UserHandler* userHandlerObject = new UserHandler(databaseObject, settings.data());
    SessionHandler* sessionHandlerObject = new SessionHandler(databaseObject, userHandlerObject);
    ImageHandler* imageHandlerObject = new ImageHandler(settings.data());
    //std::mutex *audioUdpBufferLock = new std::mutex;
    //std::mutex *videoUdpBufferLock = new std::mutex;

    InputStreamHandler* inputStreamHandler = new InputStreamHandler(imageHandlerObject, bufferSize, address);

    SocketHandler* socketHandlerObject = new SocketHandler(bufferSize,imageHandlerObject,inputStreamHandler, sessionHandlerObject, address);
    TcpSocketHandler* tcpSocketHandler = new TcpSocketHandler(inputStreamHandler, sessionHandlerObject, address, 1337);
    tcpSocketHandler->init();

    //int64_t *lastVideoPacketTime = new int64_t(-1);
    //int64_t *lastAudioPacketTime = new int64_t(-1);

    QScopedPointer<ImageHandler> imageHandler(imageHandlerObject);

    QScopedPointer<StreamHandler> streamHandler(new StreamHandler(imageHandlerObject, socketHandlerObject, bufferSize, settings.data(), tcpSocketHandler));

    //QScopedPointer<VideoPlaybackHandler> videoPlaybackHandler(new VideoPlaybackHandler(imageHandlerObject, socketHandlerObject, buffer_size, lastVideoPacketTime, lastAudioPacketTime));
    //QScopedPointer<AudioPlaybackHandler> audioPlaybackHandler(new AudioPlaybackHandler(imageHandlerObject, socketHandlerObject, buffer_size, lastVideoPacketTime, lastAudioPacketTime));
    QScopedPointer<UserHandler> userHandler(userHandlerObject);
    QScopedPointer<SessionHandler> sessionHandler(sessionHandlerObject);


    //streamHandler->record();
    //streamHandler->finish();
    //QScopedPointer<AudioHandler> audioHandler(new AudioHandler(NULL, NULL));
    engine.rootContext()->setContextProperty("imageHandler", imageHandler.data());
    engine.rootContext()->setContextProperty("sessionHandler", sessionHandler.data());
    engine.rootContext()->setContextProperty("streamHandler", streamHandler.data());
    engine.rootContext()->setContextProperty("backendSettings", settings.data());

    engine.rootContext()->setContextProperty("userHandler", userHandler.data());
    engine.addImageProvider("live", imageHandler.data());


    //QScopedPointer<VideoHandler> videoHandler(new VideoHandler("/dev/video0", NULL));
    //engine.rootContext()->setContextProperty("VideoHandler", videoHandler.data());
    engine.load(url);


    //videoHandler->init();
    //Denne klassen klarer å lagre audio stream til fil
    //QScopedPointer<AudioHandler> pureAudio(new AudioHandler(NULL,"audioHandler.ismv"));
    //pureAudio->main();

    //Denne klassen klarer å lagre video stream til fil
    //QScopedPointer<filetest> fil(new filetest);
    //fil->main();
    //exit(1337);

    //VideoHandler->init();
    //imageHandler->veryFunStianLoop();


    //playbackHandler->start();

    //AudioPlaybackHandler* ttt = new AudioPlaybackHandler();




    return app.exec();
}
