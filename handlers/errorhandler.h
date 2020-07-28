#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QObject>

class ErrorHandler : public QObject
{
    Q_OBJECT
public:
    explicit ErrorHandler(QObject *parent = nullptr);
    void giveErrorDialog(QString error)
    {
        emit showError(error);
    }
signals:
    void showError(QString error);
};

//extern QScopedPointer<ErrorHandler> ERRORHANDLER(new ErrorHandler());

extern ErrorHandler* errorHandler;


#endif // ERRORHANDLER_H
