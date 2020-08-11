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
    Q_INVOKABLE void loadAndSaveDefaultSettings(const QString& displayName);
    Q_INVOKABLE void saveSettings();
    Q_INVOKABLE void setAudioOn(bool val);
    Q_INVOKABLE void setVideoOn(bool val);
    Q_INVOKABLE void setDisplayName(const QString& val);
    Q_INVOKABLE void setDefaultAudioInput(const QString& val);
    Q_INVOKABLE void setSaveLastRoom(bool val);
    Q_INVOKABLE void setLastRoomId(const QString& val);
    Q_INVOKABLE void setLastRoomPassword(const QString& val);
    Q_INVOKABLE void setServerIpAddress(const QString &serverIpAddress);
    Q_INVOKABLE void setSqlTcpPort(int sqlTcpPort);
    Q_INVOKABLE void setUdpPort(int udpPort);
    Q_INVOKABLE void setTcpPort(int tcpPort);
    Q_INVOKABLE int getTcpPort() const;
    Q_INVOKABLE int getUdpPort() const;
    Q_INVOKABLE int getSqlTcpPort() const;
    Q_INVOKABLE bool getAudioOn() const;
    Q_INVOKABLE bool getVideoOn() const;
    Q_INVOKABLE bool getSaveLastRoom() const;
    Q_INVOKABLE QString getDisplayName() const;
    Q_INVOKABLE QString getDefaultAudioInput() const;
    Q_INVOKABLE QString getLastRoomId() const;
    Q_INVOKABLE QString getLastRoomPassword() const;
    Q_INVOKABLE QString getServerIpAddress() const;

private:
    int mTcpPort;
    int mUdpPort;
    int mSqlTcpPort;
    bool mAudioOn;
    bool mVideoOn;
    bool mSaveLastRoom;
    QString mDisplayName;
    QString mDefaultAudioInput;
    QString mSettingsFile = "./settings.json";
    QString mLastRoomId;
    QString mLastRoomPassword;
    QString mServerIpAddress;
};

#endif // SETTINGS_H
