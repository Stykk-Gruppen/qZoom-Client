#include "inputstreamhandler.h"

InputStreamHandler::InputStreamHandler(ImageHandler* imageHandler, int bufferSize, QHostAddress address, QObject *parent) : QObject(parent)
{
    mImageHandler = imageHandler;
    mBufferSize = bufferSize;
    mAddress = address;
}

void InputStreamHandler::handleHeader(QByteArray data)
{
    int streamIdLength = data[0];
    data.remove(0,1);

    //Finds the streamId header, stores it and removes it;
    QByteArray streamIdArray = QByteArray(data, streamIdLength);
    QString streamId(streamIdArray);
    data.remove(0, streamIdLength);

    int index = findStreamIdIndex(streamId);

    //Checks the first byte in the datagram to determine if the datagram is audio or video
    int audioOrVideoInt = data[0];
    data.remove(0,1);

    if(audioOrVideoInt == 0)
    {
        mAudioMutexVector[index]->lock();
        mAudioBufferVector[index]->append(data);
        mAudioMutexVector[index]->unlock();
        if(!mAudioPlaybackStartedVector[index] && mAudioBufferVector[index]->size() >= mBufferSize)
        {
            QtConcurrent::run(mAudioPlaybackHandlerVector[index], &AudioPlaybackHandler::start);
            mAudioPlaybackStartedVector[index] = true;
        }
        // qDebug() << "audio buffer size " << mAudioBufferVector[index].size() << "after signal: " << signalCount;
    }
    else if (audioOrVideoInt ==1)
    {
        mVideoMutexVector[index]->lock();
        mVideoHeaderVector[index]->append(data);
        mVideoMutexVector[index]->unlock();
        if(!mVideoPlaybackStartedVector[index] && mVideoBufferVector[index]->size() >= mBufferSize)
        {

            QtConcurrent::run(mVideoPlaybackHandlerVector[index], &VideoPlaybackHandler::start);
            mVideoPlaybackStartedVector[index] = true;
        }
        //qDebug() << "video buffer size " << mVideoBufferVector[index]->size() << "after signal: " << signalCount;
    }
    else
    {
        qDebug() << "UDP Header byte was not 1 or 0 in socketHandler, stopping program";
        exit(-1);
    }
}

void InputStreamHandler::addStreamToVector(QString streamId,int index)
{
    QByteArray* tempVideoHeaderBuffer = new QByteArray();
    QByteArray* tempAudioHeaderBuffer = new QByteArray();

    mVideoHeaderVector.push_back(tempVideoHeaderBuffer);
    QByteArray* tempAudioBuffer = new QByteArray();
    QByteArray* tempVideoBuffer = new QByteArray();
    mAudioBufferVector.push_back(tempAudioBuffer);
    mVideoBufferVector.push_back(tempVideoBuffer);
    std::mutex* tempAudioLock = new std::mutex;
    std::mutex* tempVideoLock = new std::mutex;
    mAudioMutexVector.push_back(tempAudioLock);
    mVideoMutexVector.push_back(tempVideoLock);
    mAudioPlaybackHandlerVector.push_back(new AudioPlaybackHandler(tempAudioLock,tempAudioBuffer,mBufferSize));
    mVideoPlaybackHandlerVector.push_back(new VideoPlaybackHandler(tempVideoLock,mImageHandler, tempVideoHeaderBuffer, tempVideoBuffer,mBufferSize,index+1));
    mStreamIdVector.push_back(streamId);
    mAudioPlaybackStartedVector.push_back(false);
    mVideoPlaybackStartedVector.push_back(false);

    //Your own image is at 0, so we add 1 here and in videoPlayback constructor
    mImageHandler->addPeer(index+1);
}


//Sockethandler has to put the datagram in the correct buffer when in a room with multiple people, based on the streamId
int InputStreamHandler::findStreamIdIndex(QString streamId)
{
    if(mStreamIdVector.size()>=1)
    {
        for(size_t i=0;i<mStreamIdVector.size();i++)
        {
            if(QString::compare(streamId, mStreamIdVector[i], Qt::CaseSensitive)==0)
            {
                return i;
            }
        }
        //If the streamId does not exist, push it and buffers/locks
        addStreamToVector(streamId,mStreamIdVector.size());
        return mStreamIdVector.size();
    }
    else
    {
        //If this stream is the first to join, push it and buffer/locks
        addStreamToVector(streamId,0);
        return 0;
    }

}
