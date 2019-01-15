#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QMap>
#include <QObject>
#include "logger.h"

/**
 * @brief The LogManager class is a singleton factory for Loggers.
 */

class LogManager : public QObject
{
    Q_OBJECT
public:
    LogManager();
    ~LogManager();

    Logger *getLogger(QString name);

signals:
    void newLogMessage(QString msg, MsgType type);

private:
    static LogManager* inst;
    QMap<QString, Logger *> logMap;
};

LogManager& logManager();

#endif // LOGMANAGER_H
