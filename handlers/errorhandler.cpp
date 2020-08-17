#include "errorhandler.h"
/**
 * @brief ErrorHandler::ErrorHandler
 * @param parent
 */
ErrorHandler::ErrorHandler(QObject *parent) : QObject(parent)
{

}
/**
 * @brief ErrorHandler::giveErrorDialog
 * Connected with a QML function in main.qml
 */
void ErrorHandler::giveErrorDialog(const QString& error)
{
    emit showError(error);
}
/**
 * @brief ErrorHandler::giveKickedErrorDialog
 * Connected with a QML function in main.qml
 */
void ErrorHandler::giveKickedErrorDialog()
{
    emit showKickError();
}

