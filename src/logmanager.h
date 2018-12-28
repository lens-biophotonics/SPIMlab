#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QMap>
#include <boost/signals2/signal.hpp>
#include "logger.h"

/**
 * @brief The LogManager class is a singleton factory for Loggers.
 */

typedef boost::signals2::signal<void (QString msg, MsgType type)> newLogMsg_t;

class LogManager
{
public:
    static LogManager &getInstance()
    {
        static LogManager instance; // Guaranteed to be destroyed.
                                    // Instantiated on first use.
        return instance;
    }

    newLogMsg_t newLogMessage;

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
private:
    LogManager();
    ~LogManager();

    QMap<QString, Logger *> logMap;
};

#endif // LOGMANAGER_H
