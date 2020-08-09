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

class InputStreamHandler
{
public:
    explicit InputStreamHandler(ImageHandler* imageHandler, int bufferSize, QHostAddress address);
    void addStreamToVector(int index, const QString& streamId, const QString& displayName);
    void init();
    void close();
    void removeStream(const QString& streamId);
    void updateParticipantDisplayName(const QString& streamId, const QString& displayName);
    void setPeerToVideoDisabled(const QString& streamId);
    void setPeerToAudioDisabled(const QString& streamId);
    void handleHeader(QByteArray header);
    void lockAudioMutex(int index);
    void appendToAudioBuffer(int index, const QByteArray& data);
    void unlockAudioMutex(int index);
    void lockVideoMutex(int index);
    void appendToVideoBuffer(int index, const QByteArray& data);
    void setAudioPlaybackStarted(int index, bool val);
    void unlockVideoMutex(int index);
    void setVideoPlaybackStarted(int index, bool val);
    void kickYourself();
    int findStreamIdIndex(const QString& streamId) const;
    int getAudioBufferSize(int index) const;
    int getVideoBufferSize(int index) const;
    bool audioPlaybackStarted(int index) const;
    bool videoPlaybackStarted(int index) const;
    QString getStreamIdFromIndex(int index) const;
    QFuture<void>* getAudioFutures(int index);
    QFuture<void>* getVideoFutures(int index);
    std::vector<QString> getStreamIdVector() const;
    AudioPlaybackHandler* getAudioPlaybackHandler(int index) const;
    VideoPlaybackHandler* getVideoPlaybackHandler(int index) const;

private:
    std::vector<QFuture<void>*> mAudioFutures;
    std::vector<QFuture<void>*> mVideoFutures;
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
    ImageHandler* mImageHandler;
    int mBufferSize;
    QHostAddress mAddress;

};

#endif // INPUTSTREAMHANDLER_H
