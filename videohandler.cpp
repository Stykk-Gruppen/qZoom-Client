#include "videohandler.h"

VideoHandler::VideoHandler(QObject* parent): QObject(parent)
{
    /*camera = new QCamera;
    camera->setCaptureMode(QCamera::CaptureVideo);

    videoProbe = new QVideoProbe(this);

    if (videoProbe->setSource(camera)) {
        // Probing succeeded, videoProbe->isValid() should be true.
        connect(videoProbe, SIGNAL(videoFrameProbed(QVideoFrame)),
                this, SLOT(detectBarcodes(QVideoFrame)));
    }

    camera->start();
    */

    videoProbe = new QVideoProbe;

}


void VideoHandler::setup(QObject* qmlCamera)
{
    camera = qvariant_cast<QCamera*>(qmlCamera->property("mediaObject"));



    connect(videoProbe,SIGNAL(videoFrameProbed(QVideoFrame)),this,SLOT(handleFrame(QVideoFrame)));

    videoProbe->setSource(camera);
}

void VideoHandler::handleFrame(QVideoFrame frame)
{
    qDebug() << "Handling frame\n";

    QImage img = frame.image();

    img.save("test.png");
}


