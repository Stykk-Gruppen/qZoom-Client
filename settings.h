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
    Q_INVOKABLE bool getAudioOn();
    Q_INVOKABLE bool getVideoOn();
    Q_INVOKABLE QString getDisplayName();
    Q_INVOKABLE QString getDefaultAudioInput();
    Q_INVOKABLE void setAudioOn(bool val);
    Q_INVOKABLE void setVideoOn(bool val);
    Q_INVOKABLE void setDisplayName(QString val);
    Q_INVOKABLE void setDefaultAudioInput(QString val);

private:
    bool mAudioOn;
    bool mVideoOn;
    QString mDisplayName;
    QString mDefaultAudioInput;
    QString settingsFile = "./settings.json";
};

#endif // SETTINGS_H
