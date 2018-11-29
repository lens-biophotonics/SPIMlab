#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QMap>
#include "logger.h"

/**
 * @brief The LogManager class is a singleton factory for Loggers.
 */

class LogManager
{
public:
    static LogManager &getInstance()
    {
        static LogManager instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    Logger *getLogger(QString name);
private:
    LogManager();
    ~LogManager();

// C++ 03
// Dont implement copy constructor and assignment operator
    LogManager(LogManager const &);
    void operator=(LogManager const &);

// C++ 11
#if __STDC_VERSION__ >= 201103L
public:
    LogManager(LogManager const &) = delete;
    void operator=(LogManager const &) = delete;
#endif

    QMap<QString, Logger *> logMap;
};

#endif // LOGMANAGER_H
