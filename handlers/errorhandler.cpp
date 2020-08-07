#include "errorhandler.h"

ErrorHandler::ErrorHandler(QObject *parent) : QObject(parent)
{

}

void ErrorHandler::giveErrorDialog(const QString& error)
{
    emit showError(error);
}


