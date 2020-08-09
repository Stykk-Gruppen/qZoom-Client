#ifndef VIDEOHANDLER_H
#define VIDEOHANDLER_H

#include <QUdpSocket>
#include <QObject>
#include <QtConcurrent/QtConcurrent>
#include <QGuiApplication>
#include <QScreen>
#include <string.h>
//#include <X11/Xlib.h>
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
#include "libavformat/avformat.h"
#include "libavutil/time.h"
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
}
#include "handlers/udpsockethandler.h"
#include "handlers/imagehandler.h"
#include "core/systemcall.h"
class UdpSocketHandler;
class TcpSocketHandler;

class VideoHandler : public QObject
{
    Q_OBJECT
public:
    VideoHandler(QString mCameraDeviceName, std::mutex* _writeLock,
                 int64_t mTime, ImageHandler* mImageHandler,
                 UdpSocketHandler* _socketHandler,
                 int bufferSize, TcpSocketHandler* tcpSocketHandler, bool screenShare,
                 QObject* parent = nullptr);
    ~VideoHandler();
    void grabFrames();
    void close();
    void toggleGrabFrames(bool a);
    int init();
    static int custom_io_write(void* opaque, uint8_t *buffer, int buffer_size);
    bool isActive() const;

signals:
    void callWriteHeader();



private:
    QString buildScreenDeviceName();
    //Trenger kanskje ikke denne likevel?
    struct mSocketStruct {
        UdpSocketHandler* udpSocket;
        TcpSocketHandler* tcpSocket;
        bool headerSent;
    };
    mSocketStruct* mStruct;
    struct SwsContext* img_convert_ctx;
    int mBufferSize;
    int mNumberOfSkippedFrames = 0;
    int mVideoStream;
    int mNumberOfFrames;
    int mScreenWidth;
    int mScreenHeight;
    int mStart_pts;
    int mStart_dts;
    bool mActive = false;
    bool mScreenCapture = false;
    bool mAbortGrabFrames = false;
    bool mWriteToFile = true;
    bool mIsFirstPacket = true;
    const char* mSource;
    const char* mFileName = "nyTest.ismv";
    int64_t mTime;
    int64_t mPts = 0;
    AVCodecContext* mInputVideoCodecContext;
    AVCodecContext* mOutputVideoCodecContext;
    AVFrame* mVideoFrame;
    AVFrame* mScaledFrame;
    AVFormatContext *ifmt_ctx, *ofmt_ctx;
    AVCodec* inputVideoCodec;
    AVCodec* outputVideoCodec;
    UdpSocketHandler *mSocketHandler;
    ImageHandler* mImageHandler;
    QString mAudioDeviceName;
    QString mCameraDeviceName;
    std::mutex* mWriteLock;
    TcpSocketHandler* mTcpSocketHandler;
};
#endif // VideoHandler_H
