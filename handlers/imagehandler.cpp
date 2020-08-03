#include "imagehandler.h"

ImageHandler::ImageHandler(Settings* settings) : QQuickImageProvider(QQuickImageProvider::Image)
{
    mSettings = settings;
    mDefaultImage = generateGenericImage(mSettings->getDisplayName());

    this->blockSignals(false);
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
    //qDebug() << id;
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
    //This means the screen for the user, which is stored in numeric_value_max in the map
    if(index==0)
    {
        index = std::numeric_limits<uint8_t>::max();
    }
    else
    {
        //All other participants are located in the map at their screen index-1
        index--;
    }
    QImage result = mImageMap[index].first;

    if(result.isNull())
    {
        result = generateGenericImage(mImageMap[index].second);
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
void ImageHandler::addPeer(uint8_t index, QString displayName)
{
    qDebug() << "Added peer to ImageHandler map: " << index;
    imgLock.lock();
    mImageMap[index].first = generateGenericImage(displayName);
    mImageMap[index].second = displayName;
    imgLock.unlock();
    emit refreshScreens();
}

/**
 * Removes everything from the map
 */
void ImageHandler::removeAllPeers()
{
    qDebug() << "Removing all peers";
    imgLock.lock();
    mImageMap.clear();
    imgLock.unlock();
}

/**
 * Removes the data at index from mImageMap and sends the signal refreshScreens
 * which is connected to TaskBar.qml Connections function onRefreshScreens()
 * @param index uint8_t
 */
void ImageHandler::removePeer(uint8_t index)
{
    qDebug() << "Removing peer from ImageHandler map: " << index;
    imgLock.lock();
    mImageMap.erase(index);
    imgLock.unlock();
    emit refreshScreens();
}

/**
 * Updates the data at index in the mImageMap
 * @param index uint8_t
 * @param displayName QString
 */
void ImageHandler::updatePeerDisplayName(uint8_t index, QString displayName)
{
    qDebug() << index << "Updated their display name to:" << displayName;
    mImageMap[index].second = displayName;
    mImageMap[index].first = generateGenericImage(mImageMap[index].second);
}

/**
 * If input image is not the same QImage already in the mImageMap,
 * update the mImageMap at index with the new QImage
 * @param index uint8_t
 * @param image QImage
 */
void ImageHandler::updateImage(const QImage &image, uint8_t index)
{
    imgLock.lock();
    if(mImageMap[index].first != image)
    {
        mImageMap[index].first = image;
    }
    else
    {
        qDebug() << "Same Image";
    }
    imgLock.unlock();
}

/**
 * One for the history books.
 */
void ImageHandler::veryFunStianLoop()
{
    /*QtConcurrent::run([this]()
    {
        int ms = 41; //1000/24
        int i = 1;
        QString overhead = "";
        while (true)
        {
            if (i < 100)
            {
                overhead = "0";
                if (i < 10)
                {
                    overhead = "00";
                }
            }
            else
            {
                overhead = "";
            }
            QString str = "/home/stian/Downloads/testtttt/" + overhead + QString::number(i) + ".jpg";
            QImage test = QImage(str);
            updateImage(test);
            if (i == 382)
            {
                i = 1;
            }
            i++;

            struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
            nanosleep(&ts, NULL);
        }
    });*/
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

    //avpicture_alloc((AVPicture*)frameRGB, A frame->width, frame->height);
    av_image_alloc(frameRGB->data,frameRGB->linesize,frame->width,frame->height,AV_PIX_FMT_RGB24,1);

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
            memcpy( img.scanLine(y), frameRGB->data[0]+y * frameRGB->linesize[0], frameRGB->linesize[0]);
        }
    }
    //av_freep(&frame->data[0]);
    //av_frame_unref(frame);

    av_freep(&frameRGB->data[0]);
    av_frame_unref(frameRGB);

    sws_freeContext(imgConvertCtx);

    //av_frame_free(&frameRGB);
    updateImage(img, index);
    //av_frame_unref(frameRGB);
    //av_frame_free(&frameRGB);
    //delete frameRGB;
    //sws_freeContext(imgConvertCtx);
    av_freep(&frameRGB->data[0]);

    av_frame_free(&frameRGB);

    //av_packet_unref(&packet);
}


int ImageHandler::getNumberOfScreens()
{
    return mImageMap.size();
    //return 5;
}

QImage ImageHandler::generateGenericImage(QString displayName)
{
    QImage image(QSize(1280, 720), QImage::Format_RGB32);
    QPainter painter(&image);
    painter.setBrush(QBrush(Qt::green));
    painter.fillRect(QRectF(0, 0, 1280, 720), QColor(27, 29, 54, 255));
    //painter.fillRect(QRectF(200,150,800,600), Qt::blue);

    painter.setPen(QPen(Qt::white));
    painter.setFont(QFont("Helvetica [Cronyx]", 26, QFont::Bold));
    //QString text = displayName + " hat seinen Kamera ausgeschaltet";
    QString text = displayName + "\n (Screen Disabled)";
    painter.drawText(QRectF(0, 0, 1280, 720), Qt::AlignCenter | Qt::AlignTop, text);
    return image;
}

