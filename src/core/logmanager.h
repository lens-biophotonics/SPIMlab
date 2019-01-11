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
    static LogManager* getInstance();

    // C++ 11
#if __cplusplus >= 201103L
    LogManager(LogManager const &) = delete;
    void operator=(LogManager const &) = delete;
#else
    // C++ 03
    // Dont implement copy constructor and assignment operator
    LogManager(LogManager const &);
    void operator=(LogManager const &);
#endif

    Logger *getLogger(QString name);

signals:
    void newLogMessage(QString msg, MsgType type);

private:
    LogManager();
    ~LogManager();

    static LogManager* inst;
    QMap<QString, Logger *> logMap;
};

#endif // LOGMANAGER_H
