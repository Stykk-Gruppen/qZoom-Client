#ifndef CAMERAHANDLER_H
#define CAMERAHANDLER_H

#include <QObject>
#include <QCamera>
#include <QCameraInfo>
#include <QVideoProbe>
#include <QMediaPlayer>
#include <QMediaRecorder>
#include <QObject>
#include <QVideoFrame>
#include <QCameraImageCapture>


class CameraHandler: public QObject
{
    Q_OBJECT
public:
    CameraHandler(QObject* parent = 0);
    ~CameraHandler();
    QCamera* camera;
    QString deviceId;
    QCameraImageCapture* capture;

    Q_INVOKABLE QString getDeviceId();
};

#endif // CAMERAHANDLER_H
