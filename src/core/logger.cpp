#include "logmanager.h"


Logger::Logger(QString name) : QObject(nullptr), name(name)
{
}

void Logger::msg(QString str, MsgType type) const
{
    str.prepend(QString("[%1] ").arg(name));
    emit logManager().newLogMessage(str, type);
}

void Logger::info(QString msg) const
{
    this->msg(msg);
}

void Logger::error(QString errMsg) const
{
    msg(errMsg, MSG_ERROR);
}

void Logger::critical(QString msg) const
{
    error(msg);
}

Logger *getLogger(QString name)
{
    return logManager().getLogger(name);
}
