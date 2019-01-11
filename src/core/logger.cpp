#include "logmanager.h"


Logger::Logger(QString name) : QObject(nullptr), name(name)
{
}

void Logger::msg(QString str, MsgType type)
{
    str.prepend(QString("[%1] ").arg(name));
    emit LogManager::getInstance()->newLogMessage(str, type);
}

void Logger::info(QString msg)
{
    this->msg(msg);
}

void Logger::error(QString errMsg)
{
    msg(errMsg, MSG_ERROR);
}

void Logger::critical(QString msg)
{
    error(msg);
}
