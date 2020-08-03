#include "playback.h"

Playback::Playback(std::mutex* _writeLock, QByteArray* buffer,
                   size_t bufferSize, QObject *parent) : QObject(parent)
{

    mBufferSize = bufferSize;
    mStruct = new mBufferAndLockStruct();
    mStruct->buffer = buffer;
    mStruct->writeLock = _writeLock;
    mStruct->stopPlayback = &mStopPlayback;
}
Playback::~Playback()
{
    delete mStruct;
}

void Playback::stop()
{
    mStopPlayback = true;
}

/**
 * Custom readPacket function for av_read_frame and av_open_input.
 * Will read and remove bytes from the buffer found in the struct
 * and copy them to buf.
 * @param buf_size int how many bytes to read from the buffer
 * @param[out] buf uint8_t* bytes to send back to ffmpeg
 * @param opaque void* pointer set by avio_alloc_context
 * @return buf_size int
 */
int Playback::customReadPacket(void *opaque, uint8_t *buf, int buf_size)
{
    mBufferAndLockStruct *s = reinterpret_cast<mBufferAndLockStruct*>(opaque);
    while (s->buffer->size() <= buf_size)
    {
        if((*s->stopPlayback))
        {
            return AVERROR_EOF;
        }
        //int ms = 5;
        //struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        qDebug() << "sleeping";
        //nanosleep(&ts, NULL);
    }

    s->writeLock->lock();
    QByteArray tempBuffer = QByteArray(s->buffer->data(), buf_size);
    s->buffer->remove(0,buf_size);
    s->writeLock->unlock();


    memcpy(buf, tempBuffer.constData(), buf_size);

    //Since return value is fixed it will never stop reading, should not be a problem for us?
    return buf_size;
}
