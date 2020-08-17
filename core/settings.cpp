#include "settings.h"
/**
 * @brief Settings::Settings
 */
Settings::Settings()
{
    loadSettings();
}
/**
 * Open the JSON file, read it and set all the class members.
 */
void Settings::loadSettings()
{
    QFile file(mSettingsFile);
    if (QFileInfo::exists(mSettingsFile))
    {
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        QJsonObject settings = QJsonDocument::fromJson(file.readAll()).object();
        mAudioOn = settings.value("audioOn").toBool();
        mVideoOn = settings.value("videoOn").toBool();
        mDisplayName = settings.value("displayName").toString();
        mDefaultAudioInput = settings.value("defaultAudioInput").toString();
        mSaveLastRoom = settings.value("saveLastRoom").toBool();
        mServerIpAddress = settings.value("serverIpAddress").toString();
        mTcpPort = settings.value("tcpPort").toInt();
        mUdpPort = settings.value("udpPort").toInt();
        mSqlTcpPort = settings.value("sqlTcpPort").toInt();
        if (mSaveLastRoom)
        {
            mLastRoomId = settings.value("lastRoomId").toString();
            mLastRoomPassword = settings.value("lastRoomPassword").toString();
        }
        file.close();
    }
    else
    {
        const QString displayName = "Guest" + QString::number(QDateTime::currentMSecsSinceEpoch());
        loadAndSaveDefaultSettings(displayName);
    }
}
/**
 * Applies default values to all the class member variables
 * @param displayName
 */
void Settings::loadAndSaveDefaultSettings(const QString& displayName)
{
    mAudioOn = false;
    mVideoOn = true;
    mDisplayName = displayName;
    mDefaultAudioInput = "default";
    mSaveLastRoom = true;
    mLastRoomId = "";
    mLastRoomPassword = "";
    mServerIpAddress = "Localhost";
    mUdpPort = 1337;
    mTcpPort = 1338;
    mSqlTcpPort = 1339;
    saveSettings();
}
/**
 * Opens the JSON file and overwrites the variables with the class members
 */
void Settings::saveSettings()
{
    QFile file(mSettingsFile);
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
    QJsonObject object;
    object.insert("audioOn", QJsonValue::fromVariant(mAudioOn));
    object.insert("videoOn", QJsonValue::fromVariant(mVideoOn));
    object.insert("displayName", QJsonValue::fromVariant(mDisplayName));
    object.insert("defaultAudioInput", QJsonValue::fromVariant(mDefaultAudioInput));
    object.insert("saveLastRoom", QJsonValue::fromVariant(mSaveLastRoom));
    object.insert("serverIpAddress", QJsonValue::fromVariant(mServerIpAddress));
    object.insert("tcpPort", QJsonValue::fromVariant(mTcpPort));
    object.insert("udpPort", QJsonValue::fromVariant(mUdpPort));
    object.insert("sqlTcpPort", QJsonValue::fromVariant(mSqlTcpPort));
    if (mSaveLastRoom)
    {
        object.insert("lastRoomId", QJsonValue::fromVariant(mLastRoomId));
        object.insert("lastRoomPassword", QJsonValue::fromVariant(mLastRoomPassword));
    }

    QJsonDocument settings(object);
    file.write(settings.toJson());
    file.close();
}
/**
 * @brief Settings::getAudioOn
 * @return
 */
bool Settings::getAudioOn() const
{
    return mAudioOn;
}
/**
 * @brief Settings::getVideoOn
 * @return
 */
bool Settings::getVideoOn() const
{
    return mVideoOn;
}
/**
 * @brief Settings::getDisplayName
 * @return
 */
QString Settings::getDisplayName() const
{
    return mDisplayName;
}
/**
 * @brief Settings::getDefaultAudioInput
 * @return
 */
QString Settings::getDefaultAudioInput() const
{
    return mDefaultAudioInput;
}
/**
 * @brief Settings::getLastRoomId
 * @return
 */
QString Settings::getLastRoomId() const
{
    return mLastRoomId;
}
/**
 * @brief Settings::getLastRoomPassword
 * @return
 */
QString Settings::getLastRoomPassword() const
{
    return mLastRoomPassword;
}
/**
 * @brief Settings::getTcpPort
 * @return
 */
int Settings::getTcpPort() const
{
    return mTcpPort;
}
/**
 * @brief Settings::getUdpPort
 * @return
 */
int Settings::getUdpPort() const
{
    return mUdpPort;
}
/**
 * @brief Settings::getSqlTcpPort
 * @return
 */
int Settings::getSqlTcpPort() const
{
    return mSqlTcpPort;
}
/**
 * @brief Settings::getServerIpAddress
 * @return
 */
QString Settings::getServerIpAddress() const
{
    return mServerIpAddress;
}
/**
 * @brief Settings::setServerIpAddress
 * @param serverIpAddress
 */
void Settings::setServerIpAddress(const QString &serverIpAddress)
{
    mServerIpAddress = serverIpAddress;
}
/**
 * @brief Settings::setSqlTcpPort
 * @param sqlTcpPort
 */
void Settings::setSqlTcpPort(int sqlTcpPort)
{
    mSqlTcpPort = sqlTcpPort;
}
/**
 * @brief Settings::setUdpPort
 * @param udpPort
 */
void Settings::setUdpPort(int udpPort)
{
    mUdpPort = udpPort;
}
/**
 * @brief Settings::setTcpPort
 * @param tcpPort
 */
void Settings::setTcpPort(int tcpPort)
{
    mTcpPort = tcpPort;
}
/**
 * @brief Settings::setAudioOn
 * @param val
 */
void Settings::setAudioOn(bool val)
{
    mAudioOn = val;
}
/**
 * @brief Settings::setVideoOn
 * @param val
 */
void Settings::setVideoOn(bool val)
{
    mVideoOn = val;
}
/**
 * @brief Settings::setDisplayName
 * @param val
 */
void Settings::setDisplayName(const QString& val)
{
    mDisplayName = val;
}
/**
 * @brief Settings::setDefaultAudioInput
 * @param val
 */
void Settings::setDefaultAudioInput(const QString& val)
{
    mDefaultAudioInput = val;
}
/**
 * @brief Settings::setSaveLastRoom
 * @param val
 */
void Settings::setSaveLastRoom(bool val)
{
    mSaveLastRoom = val;
}
/**
 * @brief Settings::getSaveLastRoom
 * @return
 */
bool Settings::getSaveLastRoom() const
{
    return mSaveLastRoom;
}
/**
 * @brief Settings::setLastRoomId
 * @param val
 */
void Settings::setLastRoomId(const QString& val)
{
    mLastRoomId = val;
}
/**
 * @brief Settings::setLastRoomPassword
 * @param val
 */
void Settings::setLastRoomPassword(const QString& val)
{
    mLastRoomPassword = val;
}
