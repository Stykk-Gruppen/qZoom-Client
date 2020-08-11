#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlContext>
#include <QObject>
#include "handlers/videohandler.h"
#include "handlers/audiohandler.h"
#include <QVariant>
#include <QScreen>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <QAudioInput>
#include <QTimer>
#include "handlers/outputstreamhandler.h"
#include "handlers/videoplaybackhandler.h"
#include "core/settings.h"
#include "handlers/imagehandler.h"
#include <QQuickView>
#include <stdio.h>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/log.h>
#include <libavdevice/avdevice.h>
#include <ao/ao.h>
}
#include <QtCore/QCoreApplication>
#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickView>
#include "handlers/inputstreamhandler.h"
#include "handlers/tcpsockethandler.h"
#include "handlers/audioplaybackhandler.h"
#include "handlers/sessionhandler.h"
#include "handlers/userhandler.h"
#include "handlers/errorhandler.h"

ErrorHandler* errorHandler;

/*! \mainpage qZoom-Client Documentation
 *
 * \section intro_sec Introduction
 *
 * This document describes all the C++ classes used in our qZoom Client
 *
 */

int main(int argc, char *argv[])
{
    //Registrer div ting, deprecated, men uten disse så blir det ffmpeg codec errors
    av_register_all();
    avcodec_register_all();
    avdevice_register_all();

    errorHandler = new ErrorHandler;

    //av_log_set_level(AV_LOG_QUIET);

    int bufferSize = 10e5;
    int portNumberTCP = 1338;
    int portNumberUDP = 1337;
    int portNumberTCPQueries = 1339;
    QHostAddress address;
    //address = QHostAddress::LocalHost;
    address = QHostAddress("46.250.220.57"); //tarves.no
    //address = QHostAddress("46.250.220.237"); //feqzz.no
    //address = QHostAddress::LocalHost;
    //address = QHostAddress("2001:4da8:a:1:6000:100:000f:d37b:46.250.220.57"); //tarves.no ipv6 og ipv4
    //address = QHostAddress("46.250.220.57"); //tarves.no
    //address = QHostAddress("158.36.165.235"); //Tarald
    //address = QHostAddress("92.220.136.246"); //Stian
    //address = QHostAddress("79.160.58.120"); //Kent
    //address = QHostAddress("213.162.241.177"); //KentServer

    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName("qZoom-Client");
    QCoreApplication::setApplicationVersion("1.0");
    QCommandLineParser parser;
    parser.setApplicationDescription("Hello! This is the current description");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption localHost(QStringList() << "l" << "localhost",
                                 QCoreApplication::translate("main", "Connects to localhost"));
    parser.addOption(localHost);
    parser.process(app);

    if (parser.isSet(localHost))
    {
        address = QHostAddress::LocalHost;
    }

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/view/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl)
    {
        if (!obj && url == objUrl)
        {
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);

    QScopedPointer<Settings> settings(new Settings());

    ServerTcpQueries* serverTcpQueries = new ServerTcpQueries(portNumberTCPQueries, address);
    UserHandler* userHandlerObject = new UserHandler(serverTcpQueries, settings.data());
    ImageHandler* imageHandlerObject = new ImageHandler(settings.data());
    SessionHandler* sessionHandlerObject = new SessionHandler(serverTcpQueries, userHandlerObject,
                                                              imageHandlerObject, settings.data(),
                                                              bufferSize, address, portNumberTCP,portNumberUDP);

    QScopedPointer<ImageHandler> imageHandler(imageHandlerObject);
    QScopedPointer<UserHandler> userHandler(userHandlerObject);
    QScopedPointer<SessionHandler> sessionHandler(sessionHandlerObject);

    engine.rootContext()->setContextProperty("imageHandler", imageHandler.data());
    engine.rootContext()->setContextProperty("sessionHandler", sessionHandler.data());
    engine.rootContext()->setContextProperty("backendSettings", settings.data());
    engine.rootContext()->setContextProperty("errorHandler", errorHandler);
    engine.rootContext()->setContextProperty("userHandler", userHandler.data());
    engine.addImageProvider("live", imageHandler.data());

    engine.load(url);
    return app.exec();
}
