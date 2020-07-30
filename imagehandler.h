#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QObject>
#include <QImage>
#include <QBrush>
#include <QPainter>
#include <QQuickImageProvider>
#include <QtConcurrent/QtConcurrent>
#include "settings.h"
extern "C" {
#include "libavutil/frame.h"
#include "libavformat/avformat.h"
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}


class ImageHandler : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    explicit ImageHandler(Settings* settings);
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    void veryFunStianLoop();
    void readPacket(uint8_t *buffer, int buffer_size);
    void readImage(AVCodecContext* codecContext, AVFrame* scaledFrame, uint8_t index);
    void addPeer(uint8_t index);
    void removePeer(uint8_t index);
    Q_INVOKABLE int getNumberOfScreens();
    std::mutex imgLock;

public slots:
    void updateImage(const QImage &image, uint8_t index);

signals:
    void imageChanged();
    void addScreen();
    void removeScreen();
private:
    QImage generateGenericImage(QString username);
    QMap<uint8_t, QImage> mImageMap;
    QImage mDefaultImage;
    //SwsContext *imgConvertCtx = nullptr;
    //AVFrame	*frameRGB = av_frame_alloc();
    Settings* mSettings;
};

#endif // IMAGEHANDLER_H
