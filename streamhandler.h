#ifndef STREAMHANDLER_H
#define STREAMHANDLER_H
#include "videohandler.h"
#include "audiohandler.h"
#include "sockethandler.h"

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
}
#include "imagehandler.h"

class StreamHandler
{
public:
    StreamHandler(ImageHandler* imageHandler);

    VideoHandler* videoHandler;
    AudioHandler* audioHandler;
    AVFormatContext* ofmt_ctx;
    const char* filename = "video.ismv";
    void record();
    bool writeToFile = false;
    std::mutex writeLock;
    SocketHandler* socketHandler;
    int numberOfFrames = 5000;

    static int custom_io_write(void* opaque, uint8_t *buffer, int buffer_size);

};

#endif // STREAMHANDLER_H
