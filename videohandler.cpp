#include "videohandler.h"

VideoHandler::VideoHandler(QObject* parent): QObject(parent)
{
    /*camera = new QCamera;
    camera->setCaptureMode(QCamera::CaptureVideo);

    videoProbe = new QVideoProbe(this);

    if (videoProbe->setSource(camera)) {
        // Probing succeeded, videoProbe->isValid() should be true.
        connect(videoProbe, SIGNAL(videoFrameProbed(QVideoFrame)),
                this, SLOT(detectBarcodes(QVideoFrame)));
    }

    camera->start();
    */

    videoProbe = new QVideoProbe;

}

void VideoHandler::setup(QObject* qmlCamera)
{
    camera = qvariant_cast<QCamera*>(qmlCamera->property("mediaObject"));

    connect(videoProbe,SIGNAL(videoFrameProbed(QVideoFrame)),this,SLOT(handleFrame(QVideoFrame)));

    videoProbe->setSource(camera);
}

void VideoHandler::handleFrame(QVideoFrame videoFrame)
{

    QAbstractVideoBuffer* buf = videoFrame.buffer();
    uchar* frameBits = videoFrame.bits();

    frames.push_back(frameBits);
    int ret;

    AVFrame* frame;
    AVPacket* pkt;

    AVCodec* codec;
    const char* codec_name = "mpeg";
    AVCodecContext* c = NULL;
    codec = avcodec_find_encoder_by_name(codec_name);

    c = avcodec_alloc_context3(codec);


    c->bit_rate = 400000;

    c->width = 352;
    c->height = 288;


    c->time_base = (AVRational){1, 25};
    c->framerate = (AVRational){25, 1};


    c->gop_size = 10;
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;


    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;

    // Allocate a buffer large enough for all data
    //int size = avpicture_get_size(frame->format, frame->width, frame->height);
    //uint8_t* buffer = (uint8_t*)av_malloc(size);

    // Initialize frame->linesize and frame->data pointers
    //avpicture_fill((AVPicture*)frame, frameBits, frame->format, frame->width, frame->height);

}


