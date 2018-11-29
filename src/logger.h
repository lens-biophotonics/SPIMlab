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
    MSG_ERROR = 1 /**< An error message (will be displayed in red in the log window).*/
}; /**< Type of logged message.*/

class Logger : public QObject
{
    Q_OBJECT
public:
    Logger(QString name = "");

public slots:
    void info(QString msg, MsgType type = MSG_NORMAL);
    void error(QString errMsg);
    void critical(QString msg);

private:
    QString name;
};



/**
 * @brief The event generated when a new message is sent for logging.
 */

class MessageEvent : public QEvent
{
public:
    static const QEvent::Type TYPE;

    MessageEvent(QString msg, MsgType type = MSG_NORMAL);
    virtual ~MessageEvent() {}

    MsgType getType();
    QString getMsg();

private:
    MsgType type;
    QString msg;
};

#endif // LOGGER_H
