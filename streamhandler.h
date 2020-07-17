#ifndef STREAMHANDLER_H
#define STREAMHANDLER_H
#include "videohandler.h"
#include "audiohandler.h"
#include "sockethandler.h"
#include <QObject>
extern "C" {
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavdevice/avdevice.h>
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/frame.h"
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavutil/avutil.h"
#include "libavutil/dict.h"
#include "libavutil/eval.h"
#include "libavutil/fifo.h"
#include "libavutil/pixfmt.h"
#include "libavutil/rational.h"
#include "libswresample/swresample.h"
#include "libavutil/common.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
}
#include "imagehandler.h"

class StreamHandler : public QObject
{
    Q_OBJECT
public:
    StreamHandler(ImageHandler* _imageHandler, SocketHandler* _socketHandler, QObject *parent = nullptr);
    VideoHandler* videoHandler;
    AudioHandler* audioHandler;
    void record();
    /*Q_INVOKABLE void enableAudio();
    Q_INVOKABLE void enableVideo();
    Q_INVOKABLE void disableAudio();
    Q_INVOKABLE void disableVideo();*/

private:
    SocketHandler* mSocketHandler;
    bool mAudioEnabled = true;
    bool mVideoEnabled = true;
    std::mutex mUDPSendDatagramMutexLock;
};

#endif // STREAMHANDLER_H
