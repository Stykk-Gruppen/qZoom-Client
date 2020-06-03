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
#include "camerahandler.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QScopedPointer<CameraHandler> cameraHandler(new CameraHandler);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);





    engine.rootContext()->setContextProperty("cameraHandler", cameraHandler.data());


    engine.load(url);


    cameraHandler->camera->setCaptureMode(QCamera::CaptureVideo);


    QObject::connect(cameraHandler->capture, &QCameraImageCapture::imageCaptured, [=] (int id, QImage img) {
        QByteArray buf;
        QBuffer buffer(&buf);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "PNG");
    });



    cameraHandler->camera->start();

    return app.exec();
}
