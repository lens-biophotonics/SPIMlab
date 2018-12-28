#include <QMessageBox>
#include <QMap>

#include "logmanager.h"

Logger::Logger(QString name) : QObject(NULL), name(name)
{
}

void Logger::info(QString msg, MsgType type)
{
    msg.prepend(QString("[%1] ").arg(name));
    LogManager::getInstance().newLogMessage(msg, type);
}

void Logger::error(QString errMsg)
{
    info(errMsg, MSG_ERROR);
}

void Logger::critical(QString msg)
{
    error(msg);
    QMessageBox::critical(0, "Error", msg);
}
