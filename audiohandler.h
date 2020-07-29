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
#include "sockethandler.h"

class AudioHandler
{
public:
    AudioHandler(QString cDeviceName, std::mutex* _writeLock,int64_t time,SocketHandler*, int bufferSize);
    int grabFrames();
    int init();
    void changeAudioInputDevice(QString deviceName);
    QVariantList getAudioInputDevices();
    void toggleGrabFrames(bool a);

private:
    void initPacket(AVPacket *packet);
    void cleanup();
    int mBufferSize;
    int64_t mTime;
    QString mAudioDeviceName;
    std::mutex* mWriteLock;

    int openInputFile();
    int openOutputFile();

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
    SocketHandler *mSocketHandler;
    AVFormatContext *mInputFormatContext;
    AVFormatContext *mOutputFormatContext;
    AVCodecContext *mInputCodecContext;
    AVCodecContext *mOutputCodecContext;
    SwrContext *mResampleContext;
    AVAudioFifo *mFifo;

    static int audioCustomSocketWrite(void* opaque, uint8_t *buffer, int buffer_size);
    AVDictionary *mOptions = NULL;
    bool mAbortGrabFrames = false;
};

#endif // AUDIOHANDLER_H
