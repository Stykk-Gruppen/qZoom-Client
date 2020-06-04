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
#include "videohandler.h"
#include <QCameraViewfinder>
#include <QVariant>
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);



    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);




    //QScopedPointer<VideoHandler> videoHandler(new VideoHandler);
    //QScopedPointer<CameraHandler> cameraHandler(new CameraHandler);

    //engine.rootContext()->setContextProperty("cameraHandler", cameraHandler.data());
    //qDebug() << "Camera: " << cameraHandler->getDeviceId() << "\n";

    engine.load(url);



    //cameraHandler->camera->setCaptureMode(QCamera::CaptureVideo);




    /*if(cameraHandler->capture->isReadyForCapture())
        qDebug() << "Ready!!!\n";
    else
        qDebug() << "Capture is not ready\n";

    auto* recorder = new QMediaRecorder(cameraHandler->camera);



    cameraHandler->camera->setCaptureMode(QCamera::CaptureVideo);

    /*auto&& settings = recorder->videoSettings();//6
    settings.setResolution(1280,720);
    settings.setQuality(QMultimedia::VeryHighQuality);
    settings.setFrameRate(30.0);
    recorder->setVideoSettings(settings);
*/
    /*
    recorder->setOutputLocation(QUrl::fromLocalFile("test.mp4"));
    cameraHandler->camera->start();




    qDebug() << cameraHandler->camera->status() << "\n";
    qDebug() << cameraHandler->camera->state() << "\n";
    qDebug() << cameraHandler->camera->error() << "\n";

    recorder->record();

    qDebug()<<recorder->supportedVideoCodecs() << "\n";
    qDebug()<<recorder->status() << "\n";
    qDebug()<<recorder->state() << "\n";
    qDebug()<<recorder->error() << "\n";
    qDebug()<<recorder->outputLocation() << "\n";

    /*if(cameraHandler->capture->isReadyForCapture())
        qDebug() << "Ready!!!\n";
    else
        qDebug() << "Capture is not ready\n";


    //recorder->stop();



    */

/*



    QObject *qmlCamera = engine.rootObjects().at(0)->findChild<QObject*>("qrCameraQML");

    QCamera* camera_;
    camera_ = qvariant_cast<QCamera*>(qmlCamera->property("mediaObject"));

    QVideoProbe probe_;

    connect(&probe_,SIGNAL(videoFrameProbed(QVideoFrame)),this,SLOT(handleFrame(QVideoFrame)));

    probe_.setSource(camera_)
*/



    QScopedPointer<VideoHandler> videoHandler(new VideoHandler);
    QObject *qmlCamera = engine.rootObjects().at(0)->findChild<QObject*>("qrCameraQML");

    videoHandler->setup(qmlCamera);




    return app.exec();
}
