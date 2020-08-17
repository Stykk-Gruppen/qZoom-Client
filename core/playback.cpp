#include "playback.h"
/**
 * @brief Playback::Playback
 * @param _writeLock
 * @param buffer
 * @param bufferSize
 * @param _imageHandler
 * @param index
 */
Playback::Playback(std::mutex* _writeLock, QByteArray* buffer,
                   size_t bufferSize,ImageHandler* _imageHandler, int index)
{

    mBufferSize = bufferSize;
    mStruct = new mBufferAndLockStruct();
    mStruct->buffer = buffer;
    mStruct->writeLock = _writeLock;
    mStruct->stopPlayback = &mStopPlayback;
    mIndex = index;
    mImageHandler = _imageHandler;
}
/**
 * @brief Playback::~Playback
 */
Playback::~Playback()
{
    delete mStruct;
}
/**
 * @brief Playback::stop
 */
void Playback::stop()
{
    mStopPlayback = true;
}

/**
 * Custom readPacket function for av_open_input.
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
    while(s->buffer->size() <= 0)
    {
        if((*s->stopPlayback))
        {
            return AVERROR_EOF;
        }
    }

    int stringLength = s->buffer->at(0);
    s->writeLock->lock();
    s->buffer->remove(0, 1);
    s->writeLock->unlock();

    while(s->buffer->size() <= stringLength)
    {
        if((*s->stopPlayback))
        {
            return AVERROR_EOF;
        }
    }


    QByteArray sizeArray = QByteArray(s->buffer->data(), stringLength);
    QString sizeString = QString(sizeArray);
    buf_size = sizeString.toInt();

    if(buf_size>2000 && buf_size > 0){
        s->writeLock->lock();
        s->buffer->append(stringLength);
        s->writeLock->unlock();
        return AVERROR_EOF;
    }

    while (s->buffer->size() <= buf_size+stringLength)
    {
        if((*s->stopPlayback))
        {
            return AVERROR_EOF;
        }
    }

    s->writeLock->lock();
    s->buffer->remove(0, stringLength);
    QByteArray tempBuffer = QByteArray(s->buffer->data(), buf_size);
    s->buffer->remove(0, buf_size);
    s->writeLock->unlock();

    memcpy(buf, tempBuffer.constData(), buf_size);
    return buf_size;
}
