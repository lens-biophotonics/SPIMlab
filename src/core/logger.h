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
    void msg(QString str, MsgType type = MSG_NORMAL);
    void info(QString msg);
    void error(QString errMsg);
    void critical(QString msg);

private:
    QString name;
};


#endif // LOGGER_H
