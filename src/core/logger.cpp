#include "logger.h"
#include "logmanager.h"


Logger::Logger(QString name) : QObject(nullptr), name(name)
{
}

void Logger::_msg(QString str, MsgType type) const
{
    str.prepend(QString("[%1] ").arg(name));
    emit logManager().newLogMessage(str, type);
}

void Logger::info(QString msg) const
{
    this->_msg(msg);
}

void Logger::warning(QString msg) const
{
    _msg(msg, MSG_WARNING);
}

void Logger::error(QString errMsg) const
{
    _msg(errMsg, MSG_ERROR);
}

void Logger::critical(QString msg) const
{
    error(msg);
}

Logger *getLogger(QString name)
{
    return logManager().getLogger(name);
}
