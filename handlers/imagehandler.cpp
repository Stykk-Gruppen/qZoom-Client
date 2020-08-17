#include "imagehandler.h"
/**
 * @brief ImageHandler::ImageHandler
 * @param settings
 */
ImageHandler::ImageHandler(Settings* settings) : QQuickImageProvider(QQuickImageProvider::Image)
{
    mSettings = settings;
    mDefaultImage = generateGenericImage(mSettings->getDisplayName());

    this->blockSignals(false);
}

/**
 * Will toggle the specificed QML rectangle's border on or off
 * @param talking bool
 * @param index int to which QML rectangle to change the border
 */
void ImageHandler::toggleBorder(bool talking, int index)
{
    qDebug() << "index: " << index << " talking signal: " << talking;
    mImageMap[index]->setAudioIsDisabled(false);
    mImageMap[index]->setIsTalking(talking);
}
/**
 * @brief ImageHandler::kickYourself
 * Connected with a QML function in session.qml
 */
void ImageHandler::kickYourself()
{
    emit forceLeaveSession();
}

/**
 * Returns a bool to describe if the specified participant is talking
 * @param index int to get the correct participant from the map
 * @return bool
 */
bool ImageHandler::getIsTalking(int index) const
{
    uint8_t i = getCorrectIndex(index);
    return mImageMap.at(i)->getIsTalking();
}

/**
 * Returns a bool to describe if the specified index participant muted
 * @param index int to get the correct participant from the map
 * @return bool
 */
bool ImageHandler::getAudioIsDisabled(int index) const
{
    uint8_t i = getCorrectIndex(index);
    return mImageMap.at(i)->getAudioIsDisabled();
}
/**
 * @brief ImageHandler::getAllParticipantsDisplayNames
 * @return QList of QStrings with all the participant displayNames
 */
QList<QString> ImageHandler::getAllParticipantsDisplayNames() const
{
    QList<QString> ret;
    std::map<uint8_t, Participant*>::const_iterator i;
    for (i = mImageMap.begin(); i != mImageMap.end(); i++)
    {
        ret.append(i->second->getDisplayName());
    }
    //Remove yourself
    ret.removeLast();
    return ret;
}

/**
 * The index of the QML rectangles is different than the map.
 * The client owner is stored at umeric_limits<uint8_t> in the map,
 * but is located at index 0 in the GUI, this function will bridge the
 * two different numbers together.
 * @param index QML Rectangle index
 * @return uint8_t map index
 */
uint8_t ImageHandler::getCorrectIndex(int index) const
{
    if (index == 0)
    {
        return std::numeric_limits<uint8_t>::max();
    }
    else
    {
        return (index - 1);
    }
}

/**
 * This function is run repeatedly by reload() in session.qml
 * QML will request a QImage by sending a id string. It alternates
 * between "image?id=false&0" and "image?id=false&0" where 0 is the index
 * of the screen which needs to update its QImage. This function parses the
 * string and discards everything except the index. Then it returns the
 * QImage found in the map.
 * @param id QString sent by QML
 * @param[out] size QSize how large the QImage being returned is
 * @param requestedSize QSize which size is needed by the GUI
 * @return QImage
 */
QImage ImageHandler::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    uint8_t index = 0;
    QStringList onlyId = id.split("=");
    if(onlyId.size() >= 2)
    {
        QStringList idIndex = onlyId[1].split("&");
        if(idIndex.size() >= 2)
        {

            index = idIndex[1].toUInt();
        }
    }

    index = getCorrectIndex(index);

    QImage result = mImageMap.at(index)->getImage();

    if(result.isNull())
    {
        result = generateGenericImage(mImageMap.at(index)->getDisplayName());
    }

    //Lets QML know how large the QImage is
    if(size)
    {
        *size = result.size();
    }

    //If QML sends a request with a specific size, the QImage will be rescaled before returning
    if(requestedSize.width() > 0 && requestedSize.height() > 0)
    {
        result = result.scaled(requestedSize.width(), requestedSize.height(), Qt::KeepAspectRatio);
    }

    return result;
}

/**
 * Adds data to the map at the index and sends the signal refreshScreens
 * which is connected to TaskBar.qml Connections function onRefreshScreens()
 * @param index uint8_t
 * @param displayName QString
 */
void ImageHandler::addPeer(uint8_t index, const QString& displayName)
{
    qDebug() << "Added peer to ImageHandler map: " << index << Q_FUNC_INFO;
    mImageLock.lock();
    mImageMap[index] = new Participant();
    mImageMap[index]->setImage(generateGenericImage(displayName));
    mImageMap[index]->setDisplayName(displayName);
    mImageLock.unlock();
    emit refreshScreens();
}

/**
 * Removes everything from the map
 */
void ImageHandler::removeAllPeers()
{
    qDebug() << "Removing all peers";
    mImageLock.lock();
    mImageMap.clear();
    mImageLock.unlock();
}

/**
 * Removes the data at index from mImageMap and sends the signal refreshScreens
 * which is connected to TaskBar.qml Connections function onRefreshScreens()
 * @param index uint8_t
 */
void ImageHandler::removePeer(uint8_t index)
{
    qDebug() << "Removing peer from ImageHandler map: " << index;
    mImageLock.lock();
    mImageMap.erase(index);


    qDebug() << "After Erase";
    //Might need to move the other participants down a place.
    //If we remove index 3, the map may get a gap and look like: 0, 1, 2, 4, 255
    //After the algorithm it will look like: 0, 1, 2, 3, 255
    //This has to be done to corresponds with the vectors.
    std::map<uint8_t, Participant*>::iterator i;
    uint8_t counter = 0;
    for (i = mImageMap.begin(); i != mImageMap.end(); i++)
    {
        if (i->first != counter)
        {
            if (i->first != std::numeric_limits<uint8_t>::max())
            {
                auto nodeHandler = mImageMap.extract(i);
                nodeHandler.key() = counter;
                mImageMap.insert(std::move(nodeHandler));
            }
        }
        counter++;
    }
    qDebug() << "After very fun Stian loop";
    mImageLock.unlock();
    emit refreshScreens();
}

/**
 * Updates the participant displayname at index in the mImageMap
 * @param index uint8_t
 * @param displayName QString
 */
void ImageHandler::updatePeerDisplayName(uint8_t index, const QString& displayName)
{
    qDebug() << index << "Updated their display name to:" << displayName;
    mImageMap[index]->setDisplayName(displayName);
    mImageMap[index]->setImage(generateGenericImage(mImageMap[index]->getDisplayName()));
}
/**
 * Updates the participant video muted at index in the mImageMap
 * @param index uint8_t
 */
void ImageHandler::setPeerVideoAsDisabled(uint8_t index)
{
    qDebug() << "Set peer video as disabled for index: " << index;
    mImageMap[index]->setImage(generateGenericImage(mImageMap[index]->getDisplayName()));
}
/**
 * Updates the participant audio muted at index in the mImageMap
 * @param index uint8_t
 * @param val bool
 */
void ImageHandler::setPeerAudioIsDisabled(uint8_t index, bool val)
{
    qDebug() << "Audio is disabled for index: " << index << val;
    mImageMap[index]->setAudioIsDisabled(val);
}

/**
 * If input image is not the same QImage already in the mImageMap,
 * update the mImageMap at index with the new QImage
 * @param index uint8_t
 * @param image QImage
 */
void ImageHandler::updateImage(const QImage &image, uint8_t index)
{
    mImageLock.lock();
    if(mImageMap[index]->getImage() != image)
    {
        mImageMap[index]->setImage(image);
    }
    mImageLock.unlock();
}

/**
 * We need to convert the video frame to RGB24 before turning it into a QImage object.
 * Then we update the map at said index with the new QImage.
 * @param codecContext AVCodecContext* the video decoder codec context
 * @param[in,out] frame AVFrame* which contains data for 1 video frame
 * @param index uint8_t the index in the map to update the new QImage
 */
void ImageHandler::readImage(AVCodecContext* codecContext, AVFrame* frame, uint8_t index)
{
    SwsContext *imgConvertCtx = nullptr;
    AVFrame	*frameRGB = av_frame_alloc();

    if(codecContext == nullptr)
    {
        updateImage(generateGenericImage(mSettings->getDisplayName()), std::numeric_limits<uint8_t>::max());
        return;
    }

    QImage img(frame->width, frame->height, QImage::Format_RGB888);

    frameRGB->format = AV_PIX_FMT_RGB24;
    frameRGB->width = frame->width;
    frameRGB->height = frame->height;

    av_image_alloc(frameRGB->data, frameRGB->linesize, frame->width,frame->height, AV_PIX_FMT_RGB24, 1);

    imgConvertCtx = sws_getContext(codecContext->width, codecContext->height,
                                   codecContext->pix_fmt, frame->width, frame->height, AV_PIX_FMT_RGB24,
                                   SWS_BICUBIC, NULL, NULL, NULL);
    if (imgConvertCtx)
    {
        //conversion frame to frameRGB
        sws_scale(imgConvertCtx, frame->data, frame->linesize, 0, codecContext->height, frameRGB->data, frameRGB->linesize);
        //setting QImage from frameRGB

        for (int y = 0; y < frame->height; ++y)
        {
            memcpy( img.scanLine(y), frameRGB->data[0] + y * frameRGB->linesize[0], frameRGB->linesize[0]);
        }
    }

    av_freep(&frameRGB->data[0]);
    av_frame_unref(frameRGB);

    sws_freeContext(imgConvertCtx);

    updateImage(img, index);
    av_freep(&frameRGB->data[0]);

    av_frame_free(&frameRGB);
}

/**
 * Returns how large the mImageMap is
 * @return int how many particpants in the map
 */
int ImageHandler::getNumberOfScreens() const
{
    return mImageMap.size();
}

/**
 * @brief ImageHandler::generateGenericImage
 * @param displayName
 * @return
 */
QImage ImageHandler::generateGenericImage(const QString& displayName) const
{
    QImage image(QSize(1280, 720), QImage::Format_RGB32);
    QPainter painter(&image);
    painter.setBrush(QBrush(Qt::green));
    painter.fillRect(QRectF(0, 0, 1280, 720), QColor(27, 29, 54, 255));

    painter.setPen(QPen(Qt::white));
    painter.setFont(QFont("Helvetica [Cronyx]", 26, QFont::Bold));
    QString text = displayName + "\n (Screen Disabled)";
    painter.drawText(QRectF(0, 0, 1280, 720), Qt::AlignCenter | Qt::AlignTop, text);
    return image;
}

