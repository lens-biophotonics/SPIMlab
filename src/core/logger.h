#ifndef LOGGER_H
#define LOGGER_H

#include <QEvent>
#include <QString>
#include <QObject>

/**
 * @brief Type of log message.
 */

enum MsgType {
    MSG_NORMAL = 0, /**< A normal message.*/
    MSG_ERROR = 1, /**< An error message (will be displayed in red in the log window).*/
    MSG_WARNING = 2, /**< A warning message (will be displayed in orange in the log window).*/
}; /**< Type of logged message.*/

class Logger : public QObject
{
    Q_OBJECT
public:
    Logger(QString name = "");

public slots:
    void _msg(QString str, MsgType type = MSG_NORMAL) const;
    void info(QString msg) const;
    void warning(QString msg) const;
    void error(QString errMsg) const;
    void critical(QString msg) const;

private:
    QString name;
};

Logger *getLogger(QString name);

#endif // LOGGER_H
