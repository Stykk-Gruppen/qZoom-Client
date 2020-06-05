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
#include "cameratest.h"
#include <QCameraViewfinder>
#include <QVariant>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <QAudioInput>
#include <QFile>
#include <QTimer>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
}
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









/*

    QFile destinationFile;   // Class member
    QAudioInput* audio; // Class member
    destinationFile.setFileName("/tmp/test.raw");
    destinationFile.open( QIODevice::WriteOnly | QIODevice::Truncate );

    QAudioFormat format;
    // Set up the desired format, for example:
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        format = info.nearestFormat(format);
    }
    /*
    const auto deviceInfos = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    for (const QAudioDeviceInfo &deviceInfo : deviceInfos)
        qDebug() << "Device name: " << deviceInfo.deviceName();
*/


    qDebug() << QAudioDeviceInfo::defaultInputDevice().deviceName() << "\n";


















    //QScopedPointer<VideoHandler> videoHandler(new VideoHandler);
    //QScopedPointer<CameraHandler> cameraHandler(new CameraHandler);

    //engine.rootContext()->setContextProperty("cameraHandler", cameraHandler.data());
    //qDebug() << "Camera: " << cameraHandler->getDeviceId() << "\n";

    engine.load(url);
//Integrated Camera: Integrated C
    //alsa_input.pci-0000_00_1f.3.analog-stereo
    ///dev/snd/by-path/pci-0000:00:1f.3
    QScopedPointer<CameraTest> cameraTest(new CameraTest("/dev/video0", "/dev/snd/hwC0D2"));
    cameraTest->init();
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



    ///QScopedPointer<VideoHandler> videoHandler(new VideoHandler);
    //QObject *qmlCamera = engine.rootObjects().at(0)->findChild<QObject*>("qrCameraQML");

    //videoHandler->setup(qmlCamera);



    /*QCamera* camera = new QCamera();
    QMediaRecorder* recorder = new QMediaRecorder(camera);

    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/amr");
    audioSettings.setQuality(QMultimedia::HighQuality);

    recorder->setAudioSettings(audioSettings);

    recorder->setOutputLocation(QUrl::fromLocalFile("video"));
    camera->start();

    qDebug() << camera->status() << "\n";
    qDebug() << camera->state() << "\n";
    qDebug() << camera->error() << "\n";

    recorder->record();

    qDebug()<<recorder->supportedVideoCodecs() << "\n";
    qDebug()<<recorder->status() << "\n";
    qDebug()<<recorder->state() << "\n";
    qDebug()<<recorder->error() << "\n";
    qDebug()<<recorder->outputLocation() << "\n";



    */























    return app.exec();
}
