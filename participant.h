#ifndef PARTICIPANT_H
#define PARTICIPANT_H

#include <QImage>


class Participant
{
public:
    Participant();
    void setImage(QImage val);
    void setDisplayName(QString val);
    void setIsTalking(bool val);
    void setAudioIsDisabled(bool val);
    QImage getImage();
    QString getDisplayName();
    bool getIsTalking();
    bool getAudioIsDisabled();

private:
    QImage mImage;
    QString mDisplayName;
    bool mIsTalking;
    bool mAudioIsDisabled;
};

#endif // PARTICIPANT_H
