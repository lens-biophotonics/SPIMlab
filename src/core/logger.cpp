#include "logmanager.h"


Logger::Logger(QString name) : QObject(nullptr), name(name)
{
}

void Logger::info(QString msg, MsgType type)
{
    msg.prepend(QString("[%1] ").arg(name));
    emit LogManager::getInstance()->newLogMessage(msg, type);
}

void Logger::error(QString errMsg)
{
    info(errMsg, MSG_ERROR);
}

void Logger::critical(QString msg)
{
    error(msg);
}
