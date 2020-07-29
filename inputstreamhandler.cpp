#include "inputstreamhandler.h"

InputStreamHandler::InputStreamHandler(ImageHandler* imageHandler, int bufferSize, QHostAddress address, QObject *parent) : QObject(parent)
{
    mImageHandler = imageHandler;
    mBufferSize = bufferSize;
    mAddress = address;
}

void InputStreamHandler::init()
{
    //new TcpServerHandler();
    //TcpServerHandler.init();

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

    mVideoMutexVector[index]->lock();
    //mVideoHeaderVector[index]->append(data);
    mVideoBufferVector[index]->append(data);
    mVideoMutexVector[index]->unlock();
}

/**
 * When recieving a UDP datagram, we need to know who owns the stream.
 * The index will let readPendingDatagrams know which buffer, mutex and playbackhandler to use for both audio and video.
 * @param streamId QString to find in mStreamIdVector
 * @param index int
 */
void InputStreamHandler::addStreamToVector(QString streamId,int index)
{
    qDebug() << "Adding streamId: " << streamId;
    QByteArray* tempVideoHeaderBuffer = new QByteArray();
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
    qDebug() << "InputStreamHandler findStreamId index: " << streamId;
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
        addStreamToVector(streamId, mStreamIdVector.size());
        return mStreamIdVector.size();
    }
    else
    {
        //If this stream is the first to join, push it and buffer/locks
        addStreamToVector(streamId,0);
        return 0;
    }

}
