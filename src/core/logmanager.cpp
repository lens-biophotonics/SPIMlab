#include <memory>

#include "logmanager.h"

LogManager* LogManager::inst = nullptr;

LogManager::LogManager()
{
    qRegisterMetaType<MsgType>("MsgType");
}

LogManager::~LogManager()
{
    qDeleteAll(logMap);
}

Logger *LogManager::getLogger(QString name)
{
    Logger *logger;
    QMap<QString, Logger *>::iterator it = logMap.find(name);
    if (it == logMap.end()) {
        logger = new Logger(name);
        logMap[name] = logger;
    }
    else {
        logger = it.value();
    }
    return logger;
}

LogManager &logManager()
{
    static auto instance = std::make_unique<LogManager>();
    return *instance;
}
