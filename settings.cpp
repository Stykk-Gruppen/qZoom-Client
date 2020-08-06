#include "settings.h"

Settings::Settings()
{
    loadSettings();
}

void Settings::loadSettings()
{
    QFile file(mSettingsFile);
    if(QFileInfo::exists(mSettingsFile))
    {
        qDebug() << "Found settings file";
        file.open(QIODevice::ReadWrite | QIODevice::Text);

        //QJsonDocument settings =
        QJsonObject settings = QJsonDocument::fromJson(file.readAll()).object();
        mAudioOn = settings.value("audioOn").toBool();
        mVideoOn = settings.value("videoOn").toBool();
        mDisplayName = settings.value("displayName").toString();
        mDefaultAudioInput = settings.value("defaultAudioInput").toString();
        mSaveLastRoom = settings.value("saveLastRoom").toBool();
        if (mSaveLastRoom)
        {
            mLastRoomId = settings.value("lastRoomId").toString();
            mLastRoomPassword = settings.value("lastRoomPassword").toString();
        }
        //qDebug() << mAudioOn;
        //qDebug() << mVideoOn;
        //qDebug() << mDisplayName;
        //qDebug() << mDefaultAudioInput;
        file.close();
    }
    else
    {
        qDebug() << "Settings file not found. Creating default";
        loadAndSaveDefaultSettings();
    }
}

void Settings::loadAndSaveDefaultSettings()
{
    mAudioOn = false;
    mVideoOn = true;
    mDisplayName = "Guest" + QString::number(QDateTime::currentMSecsSinceEpoch());
    mDefaultAudioInput = "default";
    mSaveLastRoom = true;
    mLastRoomId = "";
    mLastRoomPassword = "";
    saveSettings();
}

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
    if (mSaveLastRoom)
    {
        object.insert("lastRoomId", QJsonValue::fromVariant(mLastRoomId));
        object.insert("lastRoomPassword", QJsonValue::fromVariant(mLastRoomPassword));
    }

    QJsonDocument settings(object);

    //qDebug() << settings;
    file.write(settings.toJson());
    file.close();
    //qDebug() << "Saved Settings to file" << settings;
    qDebug() << "Saved Settings to file";
}

bool Settings::getAudioOn() const
{
    return mAudioOn;
}

bool Settings::getVideoOn() const
{
    return mVideoOn;
}

QString Settings::getDisplayName() const
{
    return mDisplayName;
}

QString Settings::getDefaultAudioInput() const
{
    return mDefaultAudioInput;
}

QString Settings::getLastRoomId() const
{
    return mLastRoomId;
}

QString Settings::getLastRoomPassword() const
{
    return mLastRoomPassword;
}

void Settings::setAudioOn(bool val)
{
    mAudioOn = val;
}

void Settings::setVideoOn(bool val)
{
    mVideoOn = val;
}

void Settings::setDisplayName(QString val)
{
    mDisplayName = val;
}

void Settings::setDefaultAudioInput(QString val)
{
    mDefaultAudioInput = val;
}

void Settings::setSaveLastRoom(bool val)
{
    mSaveLastRoom = val;
}

bool Settings::getSaveLastRoom() const
{
    return mSaveLastRoom;
}

void Settings::setLastRoomId(QString val)
{
    mLastRoomId = val;
}

void Settings::setLastRoomPassword(QString val)
{
    mLastRoomPassword = val;
}
