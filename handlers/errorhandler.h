#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QObject>

class ErrorHandler : public QObject
{
    Q_OBJECT
public:
    explicit ErrorHandler(QObject *parent = nullptr);
    void giveErrorDialog(const QString& error);
    void giveKickedErrorDialog();
signals:
    void showError(QString error);
    void showKickError();
};

//Makes ErrorHandler global
extern ErrorHandler* errorHandler;


#endif // ERRORHANDLER_H
