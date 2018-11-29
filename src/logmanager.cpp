#include "logmanager.h"

LogManager::LogManager()
{
}

LogManager::~LogManager()
{
    qDeleteAll(logMap);
}

Logger *LogManager::getLogger(QString name)
{
    Logger *logger;
    QMap<QString, Logger *>::iterator it = logMap.find(name);
    logger = (it == logMap.end()) ? new Logger(name) : it.value();
    return logger;
}
