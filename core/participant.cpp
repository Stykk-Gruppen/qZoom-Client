#include "participant.h"

Participant::Participant()
{
    mAudioIsDisabled = true;
    mIsTalking = false;
}
/**
 * @brief Participant::setImage
 * @param val
 */
void Participant::setImage(const QImage& val)
{
    mImage = val;
}
/**
 * @brief Participant::setDisplayName
 * @param val
 */
void Participant::setDisplayName(const QString& val)
{
    mDisplayName = val;
}
/**
 * @brief Participant::setIsTalking
 * @param val
 */
void Participant::setIsTalking(bool val)
{
    mIsTalking = val;
}
/**
 * @brief Participant::setAudioIsDisabled
 * @param val
 */
void Participant::setAudioIsDisabled(bool val)
{
    mAudioIsDisabled = val;
}
/**
 * @brief Participant::getImage
 * @return
 */
QImage Participant::getImage() const
{
    return mImage;
}
/**
 * @brief Participant::getDisplayName
 * @return
 */
QString Participant::getDisplayName() const
{
    return mDisplayName;
}

bool Participant::getIsTalking() const
{
    return mIsTalking;
}

bool Participant::getAudioIsDisabled() const
{
    return mAudioIsDisabled;
}
