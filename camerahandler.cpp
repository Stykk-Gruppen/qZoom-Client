#include "camerahandler.h"

CameraHandler::CameraHandler(QObject* parent): QObject(parent)
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if(cameras.length() > 0)
    {
        qDebug() << cameras[0].description();
        deviceId = cameras[0].deviceName();
        camera = new QCamera(cameras[0]);
        capture = new QCameraImageCapture(camera);
    }
}

CameraHandler::~CameraHandler(){}

QString CameraHandler::getDeviceId()
{
    return deviceId;
}
