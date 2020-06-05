#ifndef CAMERATEST_H
#define CAMERATEST_H

#include <QObject>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavdevice/avdevice.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
}

class CameraTest : public QObject
{
    Q_OBJECT
public:
    CameraTest(QString cDeviceName, QString aDeviceName, QObject* parent = 0);
    int init();

    void grabFrames();
    AVOutputFormat *ofmt;
    AVFormatContext *ifmt_ctx, *ofmt_ctx;

    QString cDeviceName;
    QString aDeviceName;

    int audioStream, videoStream;
    bool done;
public slots:
    void toggleDone();
};

#endif // CAMERATEST_H
