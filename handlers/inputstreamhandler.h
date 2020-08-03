#ifndef INPUTSTREAMHANDLER_H
#define INPUTSTREAMHANDLER_H

#include <QObject>
#include <mutex>
#include <vector>
#include <handlers/audioplaybackhandler.h>
#include <handlers/videoplaybackhandler.h>
#include <QHostAddress>


class VideoPlaybackHandler;
class AudioPlaybackHandler;

class InputStreamHandler : public QObject
{
    Q_OBJECT
public:
    explicit InputStreamHandler(ImageHandler* imageHandler, int bufferSize, QHostAddress address, QObject *parent = nullptr);
    void addStreamToVector(int index, QString streamId, QString displayName);
    //int findStreamIdIndex(QString streamId, QString displayName);
    int findStreamIdIndex(QString streamId);
    void init();
    void close();
    void removeStream(QString streamId);
    void updateParticipantDisplayName(QString streamId, QString displayName);
    void setPeerToVideoDisabled(QString streamId);
    void setPeerToAudioDisabled(QString streamId);
    std::vector<QByteArray*> mVideoHeaderVector;
    std::vector<QString> mStreamIdVector;
    std::vector<QByteArray*> mAudioBufferVector;
    std::vector<QByteArray*> mVideoBufferVector;
    std::vector<std::mutex*> mAudioMutexVector;
    std::vector<std::mutex*> mVideoMutexVector;
    std::vector<AudioPlaybackHandler*> mAudioPlaybackHandlerVector;
    std::vector<VideoPlaybackHandler*> mVideoPlaybackHandlerVector;
    std::vector<bool> mVideoPlaybackStartedVector;
    std::vector<bool> mAudioPlaybackStartedVector;
    void handleHeader(QByteArray header);
    ImageHandler* mImageHandler;
    int mBufferSize;
    QHostAddress mAddress;
signals:

};

#endif // INPUTSTREAMHANDLER_H
