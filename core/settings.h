#ifndef SETTINGS_H
#define SETTINGS_H
#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include "handlers/errorhandler.h"

class Settings : public QObject
{
    Q_OBJECT
public:
    Settings();
    Q_INVOKABLE void loadSettings();
    Q_INVOKABLE void loadAndSaveDefaultSettings();
    Q_INVOKABLE void saveSettings();
    Q_INVOKABLE void setAudioOn(bool val);
    Q_INVOKABLE void setVideoOn(bool val);
    Q_INVOKABLE void setDisplayName(const QString& val);
    Q_INVOKABLE void setDefaultAudioInput(const QString& val);
    Q_INVOKABLE void setSaveLastRoom(bool val);
    Q_INVOKABLE void setLastRoomId(const QString& val);
    Q_INVOKABLE void setLastRoomPassword(const QString& val);
    Q_INVOKABLE bool getAudioOn() const;
    Q_INVOKABLE bool getVideoOn() const;
    Q_INVOKABLE bool getSaveLastRoom() const;
    Q_INVOKABLE QString getDisplayName() const;
    Q_INVOKABLE QString getDefaultAudioInput() const;
    Q_INVOKABLE QString getLastRoomId() const;
    Q_INVOKABLE QString getLastRoomPassword() const;

private:
    bool mAudioOn;
    bool mVideoOn;
    bool mSaveLastRoom;
    QString mDisplayName;
    QString mDefaultAudioInput;
    QString mSettingsFile = "./settings.json";
    QString mLastRoomId;
    QString mLastRoomPassword;
};

#endif // SETTINGS_H
