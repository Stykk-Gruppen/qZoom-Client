#ifndef VIDEOHANDLER_H
#define VIDEOHANDLER_H

#include <QObject>
#include <QCamera>
#include <QCameraInfo>
#include <QVideoProbe>
#include <QMediaPlayer>
#include <QMediaRecorder>
#include <QObject>
#include <QVideoFrame>
#include <QCameraImageCapture>

class VideoHandler : public QObject
{
    Q_OBJECT
public:
    VideoHandler(QObject* parent = 0);
    QVideoProbe* videoProbe;
    QCamera* camera;
    void setup(QObject* qmlCamera);
    QMediaRecorder* recorder;

public slots:
    void handleFrame(QVideoFrame);
};

#endif // VIDEOHANDLER_H
