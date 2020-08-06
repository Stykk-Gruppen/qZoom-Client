#include "participant.h"

Participant::Participant()
{
    mAudioIsDisabled = true;
    mIsTalking = false;
}

void Participant::setImage(QImage val)
{
    mImage = val;
}

void Participant::setDisplayName(QString val)
{
    mDisplayName = val;
}

void Participant::setIsTalking(bool val)
{
    mIsTalking = val;
}

void Participant::setAudioIsDisabled(bool val)
{
    mAudioIsDisabled = val;
}

QImage Participant::getImage() const
{
    return mImage;
}

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
