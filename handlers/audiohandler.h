#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <QObject>
#include <QAudioDeviceInfo>
#include <QtConcurrent/QtConcurrent>
extern "C" {
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavcodec/avcodec.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/avassert.h"
#include "libavutil/avstring.h"
#include "libavutil/frame.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
}
#include <stdio.h>
#include "handlers/udpsockethandler.h"

class AudioHandler
{
public:
    AudioHandler(QString cDeviceName, std::mutex* _writeLock, int64_t time,
                 UdpSocketHandler*, int bufferSize ,ImageHandler *imageHandler);
    int grabFrames();
    int init();
    void changeAudioInputDevice(QString deviceName);
    QVariantList getAudioInputDevices();
    void toggleGrabFrames(bool a);
    void cleanup();
    bool isActive();

private:
    bool mActive = false;
    void initPacket(AVPacket *packet);
    int mBufferSize;
    int64_t mTime;
    QString mAudioDeviceName;
    std::mutex* mWriteLock;
    int init_filter_graph(AVFilterGraph **graph,
                          AVFilterContext **src,
                          AVFilterContext **sink);
    int openInputStream();
    int openOutputStream();

    int loadEncodeAndWrite();
    int encodeAudioFrame(AVFrame*,int*);
    int initOutputFrame(AVFrame **,int);
    int readDecodeConvertAndStore(int *);
    int convertSamples(const uint8_t **,uint8_t **, const int);
    int addSamplesToFifo(uint8_t **,const int );
    int initConvertedSamples(uint8_t ***,int);
    int decodeAudioFrame(AVFrame *,int *, int *);
    int writeOutputFileHeader();
    int initFifo();
    int initResampler();
    UdpSocketHandler *mSocketHandler;
    AVFormatContext *mInputFormatContext;
    AVFormatContext *mOutputFormatContext;
    AVCodecContext *mInputCodecContext;
    AVCodecContext *mOutputCodecContext;
    SwrContext *mResampleContext;
    AVAudioFifo *mFifo;
    ImageHandler* imageHandler;
    static int audioCustomSocketWrite(void* opaque, uint8_t *buffer, int buffer_size);
    AVDictionary *mOptions = NULL;
    bool mAbortGrabFrames = false;


    AVFilterGraph *graph;
    AVFilterContext *buffersrc_ctx, *buffersink_ctx;
    AVFrame *filt_frame = av_frame_alloc();


};

#endif // AUDIOHANDLER_H
