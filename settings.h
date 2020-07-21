#ifndef SETTINGS_H
#define SETTINGS_H
#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

class Settings : public QObject
{
    Q_OBJECT
public:
    Settings();
    Q_INVOKABLE void loadSettings();
    Q_INVOKABLE void loadAndSaveDefaultSettings();
    Q_INVOKABLE void saveSettings();
    Q_INVOKABLE bool getAudioOn() {return mAudioOn;}
    Q_INVOKABLE bool getVideoOn() {return mVideoOn;}
    Q_INVOKABLE QString getDisplayName() {return mDisplayName;}
    Q_INVOKABLE QString getDefaultAudioInput() {return mDefaultAudioInput;}
    Q_INVOKABLE void setAudioOn(bool val) {mAudioOn = val;}
    Q_INVOKABLE void setVideoOn(bool val) {mVideoOn = val;}
    Q_INVOKABLE void setDisplayName(QString val) {mDisplayName = val;}
    Q_INVOKABLE void setDefaultAudioInput(QString val) {mDefaultAudioInput = val;}

private:
    bool mAudioOn;
    bool mVideoOn;
    QString mDisplayName;
    QString mDefaultAudioInput;
    QString settingsFile = "./settings.json";

};

#endif // SETTINGS_H
