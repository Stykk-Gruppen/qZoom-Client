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
    bool getIsTalking() const;
    bool getAudioIsDisabled() const;
    QImage getImage() const;
    QString getDisplayName() const;

private:
    QImage mImage;
    QString mDisplayName;
    bool mIsTalking;
    bool mAudioIsDisabled;
};

#endif // PARTICIPANT_H
