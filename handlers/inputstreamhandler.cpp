#include "inputstreamhandler.h"

InputStreamHandler::InputStreamHandler(ImageHandler* imageHandler, int bufferSize, QHostAddress address)
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

void InputStreamHandler::close()
{
    qDebug() << "m, Closing inputStreamHandler";
    qDebug() << "Stopping playbackhandlers";

    for(size_t i = 0; i < mVideoPlaybackHandlerVector.size(); i++)
    {
        mVideoPlaybackHandlerVector.at(i)->stop();
        mVideoFutures.at(i)->waitForFinished();
    }

    for(size_t i = 0; i < mAudioPlaybackHandlerVector.size(); i++)
    {
        mAudioPlaybackHandlerVector.at(i)->stop();
        mAudioFutures.at(i)->waitForFinished();
    }

    qDebug() << "After stopping playbackhandlers";

    for(auto i : mVideoHeaderVector)
    {
        delete i;
    }

    for(auto i : mAudioBufferVector)
    {
        delete i;
    }

    for(auto i : mVideoBufferVector)
    {
        delete i;
    }

    for(auto i : mAudioMutexVector)
    {
        delete i;
    }

    for(auto i : mVideoMutexVector)
    {
        delete i;
    }

    for(auto i : mAudioPlaybackHandlerVector)
    {
        delete i;
    }

    for(auto i : mVideoPlaybackHandlerVector)
    {

        delete i;
    }

    for(auto i : mAudioFutures)
    {
        delete i;
    }

    for(auto i : mVideoFutures)
    {
        delete i;
    }

    mVideoHeaderVector.clear();
    mAudioBufferVector.clear();
    mVideoBufferVector.clear();
    mAudioMutexVector.clear();
    mVideoMutexVector.clear();
    mAudioPlaybackHandlerVector.clear();
    mVideoPlaybackHandlerVector.clear();
    mVideoFutures.clear();
    mAudioFutures.clear();
}

void InputStreamHandler::removeStream(const QString& streamId)
{
    qDebug() << "Trying to remove user with streamId: " << streamId;
    int index = findStreamIdIndex(streamId);

    /* Kan dette slettes uten at mutex er locked?
    Eg fikk en segmentation fault pga customRead
    leste av en buffer som ikke eksisterte lengre
    */
    if(index != -1)
    {
        mAudioPlaybackHandlerVector.at(index)->stop();
        mAudioFutures.at(index)->waitForFinished();

        mVideoPlaybackHandlerVector[index]->stop();
        mVideoFutures.at(index)->waitForFinished();

        mStreamIdVector.erase(mStreamIdVector.begin() + index);
        qDebug() << "Successfully removed stream with streamId: " << streamId;

        delete mVideoHeaderVector.at(index);
        delete mAudioBufferVector.at(index);
        delete mVideoBufferVector.at(index);
        delete mAudioMutexVector.at(index);
        delete mVideoMutexVector.at(index);
        delete mAudioPlaybackHandlerVector.at(index);
        delete mVideoPlaybackHandlerVector.at(index);
        mVideoHeaderVector.erase(mVideoHeaderVector.begin() + index);
        mAudioBufferVector.erase(mAudioBufferVector.begin() + index);
        mVideoBufferVector.erase(mVideoBufferVector.begin() + index);
        mAudioMutexVector.erase(mAudioMutexVector.begin() + index);
        mVideoMutexVector.erase(mVideoMutexVector.begin() + index);
        mAudioPlaybackHandlerVector.erase(mAudioPlaybackHandlerVector.begin() + index);
        mVideoPlaybackHandlerVector.erase(mVideoPlaybackHandlerVector.begin() + index);
        mAudioPlaybackStartedVector.erase(mAudioPlaybackStartedVector.begin() + index);
        mVideoPlaybackStartedVector.erase(mVideoPlaybackStartedVector.begin() + index);

        mImageHandler->removePeer(index);
        mVideoFutures.erase(mVideoFutures.begin() + index);
        mAudioFutures.erase(mAudioFutures.begin() + index);

        for(unsigned long i = index; i < mVideoPlaybackHandlerVector.size(); i++)
        {
            mVideoPlaybackHandlerVector.at(i)->decreaseIndex();
        }
    }
    else
    {
        qDebug() << "Could not find stream with streamId " << streamId << " when trying to remove it";
    }
}

/**
 * Reads the streamId from the data and removes it, then finds the matching index
 * or creates a new one and appends the header data to the appropriate buffer.
 * @param data QByteArray header data recieved from the TCP request
 */
void InputStreamHandler::handleHeader(QByteArray data)
{

    int displayNameLength = data[0];
    data.remove(0, 1);

    QByteArray displayNameArray = QByteArray(data, displayNameLength);
    QString displayName(displayNameArray);
    data.remove(0, displayNameLength);

    int streamIdLength = data[0];
    data.remove(0, 1);

    //Finds the streamId header, stores it and removes it;
    QByteArray streamIdArray = QByteArray(data, streamIdLength);
    QString streamId(streamIdArray);
    data.remove(0, streamIdLength);


    qDebug() << "Adding streamId: " << streamId;
    qDebug() << "Adding displayName: " << displayName;

    int index = findStreamIdIndex(streamId);
    if (index < 0)
    {
        //Failed to find streamId in vector. Will add them instead.
        addStreamToVector(mStreamIdVector.size(), streamId, displayName);

        index = (mStreamIdVector.size() - 1);
    }

    mVideoMutexVector[index]->lock();
    if(mVideoHeaderVector[index]->isEmpty())
    {
        qDebug() << " header is empty adding it to the buffers " << data;
        mVideoHeaderVector[index]->append(data);
        mVideoBufferVector[index]->append(data);

    }
    mVideoMutexVector[index]->unlock();
}

/**
 * When recieving a TCP request with a header from a unknown streamId,
 * we need to create new buffers, mutex and playbackhandlers for both video and audio.
 * @param streamId QString to add to mStreamIdVector, used to identify packets and datagrams
 * @param index int to add a peer to ImageHandler
 * @param displayName QString to display on GUI
 */
void InputStreamHandler::addStreamToVector(int index, const QString& streamId, const QString& displayName)
{
    qDebug() << "Adding streamId to Vectors: " << streamId;
    if(index >= std::numeric_limits<uint_8>::max())
    {
        qDebug() << "We currently do not allow for more than 255 participants in a room" << Q_FUNC_INFO;
        return;
    }
    QByteArray* tempVideoHeaderBuffer = new QByteArray();
    QFuture<void>* tempVideoFuture = new QFuture<void>();
    QFuture<void>* tempAudioFuture = new QFuture<void>();
    mVideoFutures.push_back(tempVideoFuture);
    mAudioFutures.push_back(tempAudioFuture);
    mVideoHeaderVector.push_back(tempVideoHeaderBuffer);
    QByteArray* tempAudioBuffer = new QByteArray();
    QByteArray* tempVideoBuffer = new QByteArray();
    mAudioBufferVector.push_back(tempAudioBuffer);
    mVideoBufferVector.push_back(tempVideoBuffer);
    std::mutex* tempAudioLock = new std::mutex;
    std::mutex* tempVideoLock = new std::mutex;
    mAudioMutexVector.push_back(tempAudioLock);
    mVideoMutexVector.push_back(tempVideoLock);
    mAudioPlaybackHandlerVector.push_back(new AudioPlaybackHandler(tempAudioLock, tempAudioBuffer, mBufferSize,
                                                                   mImageHandler, index));
    mVideoPlaybackHandlerVector.push_back(new VideoPlaybackHandler(tempVideoLock, tempVideoBuffer, mBufferSize,
                                                                   mImageHandler, index));
    mStreamIdVector.push_back(streamId);

    mAudioPlaybackStartedVector.push_back(false);
    mVideoPlaybackStartedVector.push_back(false);


    //Your own image is at numeric limit for uint8, so we have a problem if a room gets that many participants
    mImageHandler->addPeer(index, displayName);
}

/**
 * When recieving a TCP request we need to know if the streamId already exists in mStreamIdVector.
 * @param streamId QString to find in mStreamIdVector
 * @return int index where streamId was found, or 0 if not found
 */

/*
int InputStreamHandler::findStreamIdIndex(QString streamId, QString displayName)
{
    qDebug() << "InputStreamHandler findStreamId index: " << streamId;
    if(mStreamIdVector.size() >= 1)
    {
        for(size_t i = 0; i < mStreamIdVector.size(); i++)
        {
            if(QString::compare(streamId, mStreamIdVector[i], Qt::CaseSensitive) == 0)
            {
                return i;
            }
        }
        //If the streamId does not exist, push it and buffers/locks
        addStreamToVector(mStreamIdVector.size(), streamId, displayName);
        return mStreamIdVector.size() - 1;
    }
    else
    {
        //If this stream is the first to join, push it and buffer/locks
        addStreamToVector(0, streamId, displayName);
        return 0;
    }
}
*/
/**
 * Goes through the mStreamIdVector and locates the streamId
 * @param streamId
 * @return int index where the streamId is located in the vector
 */
int InputStreamHandler::findStreamIdIndex(const QString& streamId) const
{
    if(mStreamIdVector.size() >= 1)
    {
        for(size_t i = 0; i < mStreamIdVector.size(); i++)
        {
            if(QString::compare(streamId, mStreamIdVector[i], Qt::CaseSensitive) == 0)
            {
                return i;
            }
        }
    }
    return -1;
}

std::vector<QString> InputStreamHandler::getStreamIdVector() const
{
    return mStreamIdVector;
}

/**
 * @brief InputStreamHandler::getAudioPlaybackHandler
 * @param index
 * @return
 */
AudioPlaybackHandler* InputStreamHandler::getAudioPlaybackHandler(int index) const
{
    return mAudioPlaybackHandlerVector[index];
}

/**
 * @brief InputStreamHandler::getVideoPlaybackHandler
 * @param index
 * @return
 */
VideoPlaybackHandler* InputStreamHandler::getVideoPlaybackHandler(int index) const
{
    return mVideoPlaybackHandlerVector[index];
}

/**
 * @brief InputStreamHandler::lockAudioMutex
 * @param index
 */
void InputStreamHandler::lockAudioMutex(int index)
{
    mAudioMutexVector[index]->lock();
}

/**
 * @brief InputStreamHandler::appendToAudioBuffer
 * @param index
 * @param data
 */
void InputStreamHandler::appendToAudioBuffer(int index, const QByteArray& data)
{
    mAudioBufferVector[index]->append(data);
}

/**
 * @brief InputStreamHandler::unlockAudioMutex
 * @param index
 */
void InputStreamHandler::unlockAudioMutex(int index)
{
    mAudioMutexVector[index]->unlock();
}

void InputStreamHandler::lockVideoMutex(int index)
{
    mVideoMutexVector[index]->lock();
}

void InputStreamHandler::appendToVideoBuffer(int index, const QByteArray& data)
{
    mVideoBufferVector[index]->append(data);
}

bool InputStreamHandler::audioPlaybackStarted(int index) const
{
    return mAudioPlaybackStartedVector[index];
}

int InputStreamHandler::getAudioBufferSize(int index) const
{
    return mAudioBufferVector[index]->size();
}

void InputStreamHandler::setAudioPlaybackStarted(int index, bool val)
{
    mAudioPlaybackStartedVector[index] = val;
}

void InputStreamHandler::unlockVideoMutex(int index)
{
    mVideoMutexVector[index]->unlock();
}

bool InputStreamHandler::videoPlaybackStarted(int index) const
{
    return mVideoPlaybackStartedVector[index];
}

bool InputStreamHandler::videoHeaderVectorIsEmpty(int index) const
{
    return mVideoHeaderVector.at(index)->isEmpty();
}

QString InputStreamHandler::getStreamIdFromIndex(int index) const
{
    QString ret;
    if (mStreamIdVector.size() > static_cast<std::make_unsigned<decltype(index)>::type>(index))
    {
        return mStreamIdVector.at(index);
    }
    return ret;
}

void InputStreamHandler::setVideoPlaybackStarted(int index, bool val)
{
    mVideoPlaybackStartedVector[index] = val;
}

void InputStreamHandler::kickYourself()
{
    mImageHandler->kickYourself();
}

int InputStreamHandler::getVideoBufferSize(int index) const
{
    return mVideoBufferVector[index]->size();
}

QFuture<void>* InputStreamHandler::getAudioFutures(int index)
{
    return mAudioFutures.at(index);
}

QFuture<void> *InputStreamHandler::getVideoFutures(int index)
{
    return mVideoFutures.at(index);
}




/**
 * @brief InputStreamHandler::updateParticipantDisplayName
 * @param streamId QString
 * @param displayName
 */
void InputStreamHandler::updateParticipantDisplayName(const QString& streamId, const QString& displayName)
{
    uint8_t index = findStreamIdIndex(streamId);
    mImageHandler->updatePeerDisplayName(index, displayName);
}

void InputStreamHandler::setPeerToVideoDisabled(const QString& streamId)
{
    uint8_t index = findStreamIdIndex(streamId);

    //Stopping videoplaybackhandler

    mVideoPlaybackHandlerVector[index]->stop();
    mVideoFutures.at(index)->waitForFinished();

    mVideoMutexVector[index]->lock();
    mVideoPlaybackStartedVector[index] = false;
    mVideoBufferVector[index]->clear();
    mVideoHeaderVector[index]->clear();
    mVideoMutexVector[index]->unlock();

    mImageHandler->setPeerVideoAsDisabled(index);
}

void InputStreamHandler::setPeerToAudioDisabled(const QString& streamId)
{
    uint8_t index = findStreamIdIndex(streamId);

    //Stopping audioPlaybackhandler:

    mAudioPlaybackHandlerVector[index]->stop();
    mAudioFutures.at(index)->waitForFinished();
    mAudioMutexVector[index]->lock();
    mAudioPlaybackStartedVector[index] = false;
    mAudioBufferVector[index]->clear();
    mAudioMutexVector[index]->unlock();

    mImageHandler->setPeerAudioIsDisabled(index, true);
}




