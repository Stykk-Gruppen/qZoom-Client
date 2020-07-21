#include "settings.h"

Settings::Settings()
{
    loadSettings();
}

void Settings::loadSettings()
{
    QFile file(settingsFile);
    if(QFileInfo::exists(settingsFile))
    {
        qDebug() << "Found settings file";
        file.open(QIODevice::ReadWrite | QIODevice::Text);

        //QJsonDocument settings =
        QJsonObject settings = QJsonDocument::fromJson(file.readAll()).object();
        mAudioOn = settings.value("audioOn").toBool();
        mVideoOn = settings.value("videoOn").toBool();
        mDisplayName = settings.value("displayName").toString();
        mDefaultAudioInput = settings.value("defaultAudioInput").toString();
        qDebug() << mAudioOn;
        qDebug() << mVideoOn;
        qDebug() << mDisplayName;
        qDebug() << mDefaultAudioInput;
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
    mDisplayName = "Username";
    mDefaultAudioInput = "default";

    saveSettings();
}

void Settings::saveSettings()
{
    QFile file(settingsFile);

    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
    QJsonObject object;
    object.insert("audioOn", QJsonValue::fromVariant(mAudioOn));
    object.insert("videoOn", QJsonValue::fromVariant(mVideoOn));
    object.insert("displayName", QJsonValue::fromVariant(mDisplayName));
    object.insert("defaultAudioInput", QJsonValue::fromVariant(mDefaultAudioInput));

    QJsonDocument settings(object);

    qDebug() << settings;
    file.write(settings.toJson());
    file.close();
    qDebug() << "Saved Settings to file";
}
